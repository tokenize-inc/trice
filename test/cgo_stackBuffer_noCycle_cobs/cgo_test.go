package cgot

import (
	"bytes"
	"io"
	"testing"

	"github.com/rokath/trice/internal/args"
	"github.com/spf13/afero"
	"github.com/tj/assert"
)

func TestLogs(t *testing.T) {

	// triceLog is the log function for executing the trice logging on binary log data in buffer as space separated numbers.
	// It uses the inside fSys specified til.json and returns the log output.
	triceLog := func(t *testing.T, fSys *afero.Afero, buffer string) string {
		var o bytes.Buffer
		assert.Nil(t, args.Handler(io.Writer(&o), fSys, []string{"trice", "log", "-i", testDataDir + "/til.json", "-p", "BUFFER", "-args", buffer, "-packageFraming", "COBS", "-hs", "off", "-prefix", "off", "-ts0", "time:        ", "-ts16", "time:%8x", "-ts32", "time:%8x", "-li", "off", "-color", "off"}))
		return o.String()
	}

	triceLogTest(t, triceLog)
}
