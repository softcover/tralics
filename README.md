# Tralics

Tralics is a LaTeX-to-XML translator developed by [Inria](http://en.wikipedia.org/wiki/French_Institute_for_Research_in_Computer_Science_and_Automation). This is a fork of Tralics used by [Softcover](http://softcover.io/).

## Installation

The Softcover ebook production system is distributed via the Ruby gems `softcover` and `polytexnic`. The `polytexnic` gem ships with precompiled Tralics binaries for OS X and select Linux distributions, so that `polytexnic` Just Works™ on most systems. But if your system is not among those supported natively, you should compile a version of Tralics for your system and place the resulting excutable on the path:

    $ git clone https://github.com/softcover/tralics
    $ cd tralics/src
    $ make
    $ cp tralics /usr/local/bin

In place of `/usr/local/bin` in the last step, you can choose any directory on your path.
