FROM frime/qtenv

#### BUILD 
RUN mkdir /root/src
COPY . /root/src
RUN mkdir /root/build
RUN cd /root/build && qmake /root/src/quickhub.pro && make -j4   
EXPOSE 4711
EXPOSE 8080
ENV LD_LIBRARY_PATH="/root/build/bin/lib:/root/build/bin/plugins"
ENTRYPOINT ["./root/build/bin/run", "-f", "/quickhub/"]

