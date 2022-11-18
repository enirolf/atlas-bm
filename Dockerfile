FROM atlas/analysisbase:22.2.97 AS atlas-bm

USER atlas

WORKDIR /home/atlas

RUN mkdir ./build

COPY ./setup_docker.sh .
COPY ./src ./src
COPY ./data ./data

RUN sudo chown -R atlas ./src
RUN sudo chown -R atlas ./data

