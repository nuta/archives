FROM ubuntu:16.04
RUN apt-get update && apt-get install -qy zsh git build-essential qemu python3 wget tree tmux
ENV HOME /root
WORKDIR /root
RUN git clone https://github.com/seiyanuta/dotfiles .dotfiles
RUN cd .dotfiles && ./setup
RUN chsh -s /bin/zsh
ENV SHELL /bin/zsh
CMD ["zsh"]