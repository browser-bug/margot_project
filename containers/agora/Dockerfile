FROM gcc:latest

RUN apt-get update && apt-get install -y cmake wget libgtest-dev libssl-dev && rm -rf /var/lib/apt/lists/*

RUN cd /home && wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz \
  && tar xfz boost_1_60_0.tar.gz \
  && rm boost_1_60_0.tar.gz \
  && cd boost_1_60_0 \
  && ./bootstrap.sh --with-libraries=program_options \
  && ./b2 install \
  && cd /home \
  && rm -rf boost_1_60_0

ENV LD_LIBRARY_PATH=/usr/local/lib
ENV margot_heel_generator_DIR=/usr/local/lib/cmake/margot

WORKDIR /usr/src/margot
RUN cd /usr/src/margot \
  && git clone https://github.com/eclipse/paho.mqtt.c \
  && cd paho.mqtt.c \ 
  && mkdir build \
  && cd build \
  && cmake .. \
  && make install \
  && cd /usr/src/margot  

RUN cd /usr/src/margot \
  && git clone https://gitlab.com/margot_project/core.git \
  && cd core \
  && mkdir build \
  && cd build \
  && cmake .. \
  && make install \
  && cd ../..

ENTRYPOINT [ "agora" ]
CMD ["--workspace-directory /usr/src/work/run --plugin-directory /usr/src/margot/core/agora/plugins --models-directory /usr/src/work/run/models/ --qos 0 --storage-address /usr/src/work --min-log-level debug"]