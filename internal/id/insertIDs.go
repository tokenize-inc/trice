// Copyright 2020 Thomas.Hoehenleitner [at] seerose.net
// Use of this source code is governed by a license that can be found in the LICENSE file.

package id

// source tree management

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"

	"github.com/rokath/trice/pkg/ant"
	"github.com/spf13/afero"
)

// SubCmdIdInsert performs sub-command insert, adding trice IDs to source tree.
func SubCmdIdInsert(w io.Writer, fSys *afero.Afero) error {
	return cmdManageTriceIDs(w, fSys, triceIDInsertion)
}

// triceIDInsertion reads file, processes it and writes it back, if needed.
func triceIDInsertion(w io.Writer, fSys *afero.Afero, path string, fileInfo os.FileInfo, a *ant.Admin) error {

	in, err := fSys.ReadFile(path)
	if err != nil {
		return err
	}
	if Verbose {
		fmt.Fprintln(w, path)
	}

	out, fileModified, err := insertTriceIDs(w, path, in, a)
	if err != nil {
		return err
	}

	if fileModified && !DryRun {
		if Verbose {
			fmt.Fprintln(w, "Changed: ", path)
		}
		err = fSys.WriteFile(path, out, fileInfo.Mode())
	}
	return err
}

// insertTriceIDs does the ID insertion task on in. insertTriceIDs uses internally local pointer idd because idd cannot be easily passed via parameters.
// insertTriceIDs returns the result in out with modified==true when out != in.
// in is the read file path content and out is the file content which needs to be written.
// a is used for mutex access to idd data. path is needed for location information.
// insertTriceIDs is intended to be used in several Go routines (one for each file) for faster ID insertion.
// Data usage:
// - idd.idToTrice is the serialized til.json. It is extended with unknown and new IDs and written back to til.json finally.
// - idd.triceToId is the initially reverted idd.idToTrice. It is shrunk for each used ID amd used to find out if an ID is already fresh used.
//   - When starting, idd.triceToId holds all IDs from til.json and no ID is fresh used yet. If an ID is to be (fresh) used it is removed from idd.triceToId.
//   - If an ID is found in idd.idToTrice but not found in idd.triceToId anymore, it is already (fresh) used and not usable again.
//   - If a new ID is generated, it is added to idd.idToTrice only. This way it gets automatically used.
//
// - idd.idToLocRef is only for reference and not changed. It is the "old" location information.
// - idd.idToLocNew is new generated during insertTriceIDs execution and finally written back to li.json as "new" location information.
// For reference look into file TriceUserGuide.md part "The `trice insert` Algorithm".
// insertTriceIDs parses the file content from the beginning for the next trice statement, deals with it and continues until the file content end.
// When a trice statement was found, general cases are:
// - idInSourceIsNonZero, id is inside idd.idToTrice with matching trice and inside idd.triceToId -> use ID (remove from idd.triceToId)
//   - If trice is assigned to several IDs, the location information consulted. If a matching path exists, its first occurrence is used.
//
// - idInSourceIsNonZero, id is inside idd.idToTrice with matching trice and not in idd.triceToId -> used ID! -> create new ID && invalidate ID in source
// - idInSourceIsNonZero, id is inside idd.idToTrice with different trice                         -> used ID! -> create new ID && invalidate ID in source
// - idInSourceIsNonZero, id is not inside idd.idToTrice (cannot be inside idd.triceToId)         -> add ID to idd.idToTrice
// - idInSourceIsZero,    trice is not inside idd.triceToId                                       -> create new ID & add ID to idd.idToTrice
// - idInSourceIsZero,    trice is is inside idd.triceToId                                        -> unused ID -> use ID (remove from idd.triceToId)
//   - If trice is assigned to several IDs, the location information consulted. If a matching path exists, its first occurrence is used.
func insertTriceIDs(w io.Writer, path string, in []byte, a *ant.Admin) (out []byte, modified bool, err error) {
	var idn TriceID    // idn is the last found id inside the source.
	var idN TriceID    // idN is the to be written id into the source.
	var idS string     // idS is the "iD(n)" statement, if found.
	rest := string(in) // rest is the so far not processed part of the file.
	outs := rest       // outs is the resulting string.
	var offset int     // offset is incremented by n, when rest is reduced by n.
	var t TriceFmt     // t is the actual located trice.
	line := 1          // line counts source code lines, these start with 1.
	for {
		idn = 0                 // clear here
		idN = 0                 // clear here
		loc := matchTrice(rest) // loc is the position of the next trice type (statement name with opening parenthesis followed by a format string).
		if loc == nil {
			break // done
		}
		t.Type = rest[loc[0]:loc[1]]       // t.Type is the TRice8_2 or TRice part for example. Hint: TRice defaults to 32 bit if not configured differently.
		t.Strg = rest[loc[5]+1 : loc[6]-1] // Now we have the complete trice t (Type and Strg). We remove the double quotes wit +1 and -1.
		if loc[3] != loc[4] { // iD(n) found
			idS = rest[loc[3]:loc[4]] // idS is where we expect n.
			nLoc := matchNb.FindStringIndex(idS)
			if nLoc == nil { // Someone wrote trice( iD(0x100), ...), trice( id(), ... ) or trice( iD(name), ...) for example.
				if Verbose {
					fmt.Fprintln(w, "unexpected syntax", idS)
				}
				line += strings.Count(rest[:loc[6]], "\n") // Keep line number up-to-date for location information.
				rest = rest[loc[6]:]
				offset += loc[6]
				continue // ignore such cases
			} else { // This is the normal case like trice( iD( 111)... .
				nStrg := idS[nLoc[0]:nLoc[1]] // nStrg is the plain number string.
				n, err := strconv.Atoi(nStrg)
				if err == nil {
					idn = TriceID(n) // idn is the assigned id inside source file.
				} else { // unexpected
					fmt.Fprintln(w, err, nStrg)                // report
					line += strings.Count(rest[:loc[6]], "\n") // Keep line number up-to-date for location information.
					rest = rest[loc[6]:]
					offset += loc[6]
					continue // ignore such cases
				}
			}
		}
		// trice t (t.Type & t.Strg) is known now. idn holds the trice id found in the source. Example cases are:
		// - trice( "foo", ... );           --> idn =   0, loc[3] == loc[4]
		// - trice( iD(0), "foo, ... ")     --> idn =   0, loc[3] != loc[4]
		// - trice( iD(111), "foo, ... ")   --> idn = 111, loc[3] != loc[4]
		a.Mutex.Lock()                       // several files could contain the same t
		if ids, ok := idd.triceToId[t]; ok { // t has at least one unused ID, but it could be from a different file.
			for i, id := range ids { // It is also possible, that no id matches idn != 0.
				if li, ok := idd.idToLocRef[id]; ok && li.File == path && (idn == 0 || idn == id) {
					// id exists inside location information for this file and is usable, so remove from unused list.
					ids[i] = ids[len(ids)-1] // todo: avoid inversion!
					ids = ids[:len(ids)-1]
					if len(ids) == 0 {
						delete(idd.triceToId, t)
					} else {
						idd.triceToId[t] = ids
					}
					idN = id // This gets into the source. No need to remove id from idd.idToLocRef.
					goto idUsable
				}
				// The case idn != 0 and idn != id is possible, when idn was manually written into the code or code with IDs was merged.
				// It is not expected, that in such cases idn is found inside idd.idToLocRef. Example:
				// TRice( iD(3), "foo" ) in file1.c && t{TRice, "foo"} gives []int{1,2}
				// li.json could contain ID 3 for file1.c, but that must be for a different trice then.
				// Therefore such idn are discarded by not copying them to idN.
			}
		}
		if idN == 0 { // create a new ID
			idN = idd.newID()
			idd.idToTrice[idN] = t // add ID to idd.idToTrice
		}
	idUsable:
		a.Mutex.Unlock()
		line += strings.Count(rest[:loc[1]], "\n") // Update line number for location information.
		if idN != idn {                            // Need to change source.
			outs = writeID(outs, offset, loc, t, idN)
			modified = true
		}
		idd.idToLocNew[idN] = TriceLI{path, line}        // Add to new location information.
		line += strings.Count(rest[loc[1]:loc[6]], "\n") // Keep line number up-to-date for location information.
		rest = rest[loc[6]:]
		offset += loc[6]
	}
	out = []byte(outs)
	return
}

// writeID inserts id into s according to loc information
func writeID(s string, offset int, loc []int, t TriceFmt, id TriceID) string {
	var idName string
	if t.Type[2] == 'i' { // small letter
		idName = "iD"
	} else {
		if DefaultStampSize == 32 {
			idName = "ID"
		} else if DefaultStampSize == 16 {
			idName = "Id"
		} else {
			idName = "id"
		}
	}
	// Example:
	// `break; case __LINE__: trice(iD(999), "msg:value=%d\n", -1  );`
	// loc:                   0   123     4  5              6
	result := s[:offset+loc[2]] + idName + "(" + strconv.Itoa(int(id)) + ")" + s[offset+loc[4]:]
	return result
	//  rest = strings.Replace(rest, idOld, idNew, 1)
}

// stringLiterals is explained in https://stackoverflow.com/questions/76587323.
var stringLiterals bufio.SplitFunc = func(data []byte, atEOF bool) (advance int, token []byte, err error) {
	scanning := false
	var delim byte
	var i int
	var start, end int
	for i < len(data) {
		b := data[i]
		switch b {
		case '\\': // skip escape sequences
			i += 2
			continue
		case '"':
			fallthrough
		case '\'':
			fallthrough
		case '`':
			if scanning && delim == b {
				end = i + 1
				token = data[start:end]
				advance = end
				return
			} else if !scanning {
				scanning = true
				start = i
				delim = b
			}
		}
		i++
	}
	if atEOF {
		return len(data), nil, nil
	}
	return start, nil, nil
}

// matchFormatString returns a two-element slice of integers defining the location of the leftmost match in s of the matchFmtString regular expression.
// The match itself is at s[loc[0]:loc[1]]. A return value of nil indicates no match.
// If the format string contains `\"` elements, the found sub strings are concatenated to the returned result.
func matchFormatString(input string) (loc []int) {
	scanner := bufio.NewScanner(strings.NewReader(input))
	scanner.Split(stringLiterals)
	if scanner.Scan() {
		loc = append(loc, strings.Index(input, scanner.Text()))
		loc = append(loc, loc[0]+len(scanner.Text()))
	}
	return
}