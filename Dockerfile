FROM atlas/analysisbase:22.2.98 AS atlas-bm

WORKDIR /home/atlas

COPY ./setup_docker.sh .
COPY ./src ./src
COPY ./data ./data
COPY ./output ./output

USER atlas

