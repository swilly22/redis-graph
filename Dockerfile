FROM redislabsmodules/rmbuilder:latest as builder

# Set up a build environment
RUN set -ex;\
    pip install redisgraph;

# Build the source
ADD ./ /redisgraph
WORKDIR /redisgraph
RUN set -ex;\
    make clean; \
    make all -j 4; \
    make test;

# Package the runner
FROM redis:latest
ENV LIBDIR /usr/lib/redis/modules
WORKDIR /data
RUN set -ex;\
    mkdir -p "$LIBDIR";
COPY --from=builder /redisgraph/src/redisgraph.so "$LIBDIR"

CMD ["redis-server", "--loadmodule", "/usr/lib/redis/modules/redisgraph.so"]
