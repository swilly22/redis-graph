# Testing RedisGraph


## Using Docker

RedisGraph's `Dockerfile` uses a recent verion of Docker in order to take
advantage of features such as [build stages](https://docs.docker.com/develop/develop-images/multistage-build/).
To build RedisGraph Docker images, you will need a version of Docker equal to
or greater than 17.05. To update the Docker on your system, see the
[Docker update documentation](https://www.docker.com/community-edition#/download)
for your OS.

The RedisGraph Docker build has been updated to run the tests after compiling
the module, so running the tests with Docker is simply a matter of executing
the following:

```
$ make docker
```

As the output scrolls by, towards the end you will see the tests get run, and
if all goes as expected, you will see them all pass.


## Native Mode

If you'd like to test RedisGraph without using Docker, you will essentially
need to duplicate what the RedisGraph `Dockerfile` does:

1. Install `make` and associated build tools (on a Debian-based system
   this is done with `apt-get install build-essential`).
1. Install `python` and `pip`.
1. Install the Redis Python client and the `rmtest` library.

With these in place, you should be able to execute the following in the
`redis-graph` directory:

```
$ make test
```
