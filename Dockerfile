ARG CI_REGISTRY=registry.izhpt.com:443

FROM $CI_REGISTRY/ayratproject/stm_builder:main AS builder

ARG BUILD_VERSION="v0.0.0"
ARG BUILD_DATE=""

ENV APP_ROOT=/app
ENV SRC_ROOT=$APP_ROOT/src
ENV BUILD_ROOT=$APP_ROOT/build

ADD . $SRC_ROOT
RUN mkdir -p $BUILD_ROOT \
 && cp -f $APP_ROOT/CMakeLists.txt $SRC_ROOT/CMakeLists.txt \
 && cp -f $APP_ROOT/search.cmake $SRC_ROOT/search.cmake

RUN cd $BUILD_ROOT \
 && cmake -G"Unix Makefiles" $SRC_ROOT --log-level=STATUS -DDEBUG=1 -DPROJECT_NAME=firmware -DBUILD_VERSION="$BUILD_VERSION" \
 && cmake --build $BUILD_ROOT # > /dev/null 2>&1

FROM scratch

ENV APP_ROOT=/app
ENV SRC_ROOT=$APP_ROOT/src
ENV BUILD_ROOT=$APP_ROOT/build

COPY --from=builder $BUILD_ROOT/firmware.bin /
COPY --from=builder $BUILD_ROOT/firmware.elf /


