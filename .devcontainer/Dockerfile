FROM ubuntu@sha256:79efa276fdefa2ee3911db29b0608f8c0561c347ec3f4d4139980d43b168d991

# Update the package database and install required tools
RUN apt update
RUN apt install -y python3 python3-pip pipx git less vim sudo cmake make g++-15 clang-format clangd-19 nodejs npm

# Update  gcc/g++ version to the latest ones
RUN ln -fs /usr/bin/g++-15 /usr/bin/g++
RUN ln -fs /usr/bin/gcc-15 /usr/bin/gcc

# Create a non-root user 'dev'
RUN useradd -ms /bin/bash dev \
    && echo "dev ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers.d/dev \
    && chmod 0440 /etc/sudoers.d/dev

RUN git clone https://github.com/tree-sitter/tree-sitter
WORKDIR tree-sitter
RUN ls -la
RUN make -j4
RUN sudo make install
RUN apt install -y cargo
RUN cargo install tree-sitter-cli --root /usr/local
ENV PATH="/root/.cargo/bin:${PATH}"
WORKDIR ..
RUN git clone https://github.com/tree-sitter/tree-sitter-python
WORKDIR tree-sitter-python/
RUN ls -la
RUN tree-sitter init-config
RUN tree-sitter generate
RUN gcc -shared -o libtree-sitter-python.so -Isrc src/parser.c -fPIC
RUN sudo mkdir -p ~/.tree-sitter/bin
RUN sudo ln -s $(pwd)/libtree-sitter-python.so ~/.tree-sitter/bin/

WORKDIR ..
RUN echo '{"parser-directories":["/"]}' > /root/.config/tree-sitter/config.json

RUN chmod 777 /root/.config/tree-sitter/config.json
RUN chmod 755 /root
RUN chmod 755 /root/.config
RUN chmod 755 /root/.config/tree-sitter
RUN chmod 644 /root/.config/tree-sitter/config.json

# Switch to the 'dev' user
USER dev

RUN ls /root/.config/tree-sitter/config.json 

RUN git config --global core.editor code

# Install conan through pipx
RUN pipx install conan
RUN pipx ensurepath

# Copy conan profile
COPY default /home/dev/.conan2/profiles/default
RUN sudo chown -R dev:dev /home/dev/.conan2
RUN sudo chmod 666 /home/dev/.conan2/profiles/default

# Set the default command
CMD ["/bin/bash"]