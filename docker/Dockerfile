FROM golang:1.20.1-alpine3.17 as build

WORKDIR /usr/src/app

# pre-copy/cache go.mod for pre-downloading dependencies and only redownloading
# them in subsequent builds if they change
COPY go.mod go.sum ./
RUN go mod download && go mod verify

COPY . .
RUN go build -buildvcs=false -v -o /usr/local/bin ./...

FROM alpine:3.17

COPY --from=build /usr/local/bin/* /usr/local/bin/

ENTRYPOINT ["trice"]
