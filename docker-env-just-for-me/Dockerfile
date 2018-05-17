FROM ubuntu:18.04
RUN \
  apt-get update && \
  apt-get install -qy clang mtools vim sudo zsh git build-essential qemu python3 curl wget tree tmux && \
  curl -sL https://deb.nodesource.com/setup_10.x | bash - && \
  curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add - && \
  echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list && \
  apt-get update && \
  apt-get install -y nodejs yarn
ENV HOME /root
WORKDIR /root
RUN git clone https://github.com/seiyanuta/dotfiles .dotfiles
RUN cd .dotfiles && ./setup
RUN git clone https://github.com/resea/resea
RUN chsh -s /bin/zsh
ENV SHELL /bin/zsh
CMD ["zsh"]