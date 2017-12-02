FROM ubuntu:16.04
RUN apt-get update && apt-get install -qy sudo zsh git build-essential qemu python3 curl wget tree tmux
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash -
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
RUN echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
RUN apt-get install -y nodejs yarn
ENV HOME /root
WORKDIR /root
RUN git clone https://github.com/seiyanuta/dotfiles .dotfiles
RUN cd .dotfiles && ./setup
RUN chsh -s /bin/zsh
ENV SHELL /bin/zsh
CMD ["zsh"]