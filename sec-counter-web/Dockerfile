FROM registry.opensuse.org/opensuse/tumbleweed:latest AS build-stage
RUN zypper install --no-recommends --auto-agree-with-product-licenses -y git go make
RUN git clone https://github.com/thkukuk/survive-soft-reboot
RUN cd survive-soft-reboot/sec-counter-web && make update && make tidy && make

FROM registry.opensuse.org/opensuse/busybox:latest
LABEL maintainer="Thorsten Kukuk <kukuk@thkukuk.de>"

ARG BUILDTIME=
ARG VERSION=unreleased
LABEL org.opencontainers.image.title="Hello World Seconds Counter"
LABEL org.opencontainers.image.description="Container containing a Hello World application which counts the seconds since it got started and refreshes the page every second."
LABEL org.opencontainers.image.created=$BUILDTIME
LABEL org.opencontainers.image.version=$VERSION

COPY --from=build-stage /survive-soft-reboot/sec-counter-web/bin/sec-counter-web /usr/local/bin/hello-world
COPY --from=build-stage /survive-soft-reboot/sec-counter-web/webpage/ /usr/local/share/hello-world/

EXPOSE 8080/tcp

CMD ["/usr/local/bin/hello-world", "-d", "/usr/local/share/hello-world"]
