# qping
Quickly connect to a URL to check your internet status

## Compiling and Running
Use the included makefile; `make qping` to compile, then `./qping` to run.

You can also use `make qp` so that compiles with a shorter name and thus can be ran quicker via `./qp`.

## Usage
Running qping without any arguments will connect to a website and download what's located there to heap memory. 

If no errors occur, it means your internet connection is fine (except if the data at the provided URL is too much for heap memory).

Optional command line arguments can also be used:
* -h: Displays a help page
* -u [URL]: Connects to URL provided in [URL] rather than the default ([a secret very fun link](https://en.wikipedia.org/wiki/Cat))
* -c: Turns the program to connect only mode; it only confirms that a connection can be made to the URL, and doesn't download what's there.
