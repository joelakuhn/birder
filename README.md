# Birder

`birder` is a simple tool for watching changes to a file system, much like Facebook's watchman utility, just way way simpler. It uses the lovely inotify for linux systems and the incomprehensibly overengineered File System Events API on MacOS X, keeping resource usage low.

## Usage

```
Usage: birder [flags] paths... -- command
    -a, --append             appends changed file name to command
    -g, --glob               treat paths as globs
    -d, --daemonize          launch as daemon
    -h, --help               print help message
```
