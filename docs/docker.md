# Docker


## Using Docker

### Build Docker image

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

### Running within a Docker container

To run RedisGraph within docker simply execute:
```
$ docker run -p 6379:6379 redislabs/redisgraph
```
