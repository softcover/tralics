# Tralics

Tralics is a LaTeX-to-XML translator developed by [Inria](http://en.wikipedia.org/wiki/French_Institute_for_Research_in_Computer_Science_and_Automation). This is a fork of Tralics used by [Softcover](http://softcover.io/).

## Installation

The Softcover ebook production system is distributed via the Ruby gems `softcover` and `polytexnic`, the latter of which uses Tralics to convert LaTeX to XML (and [thence](http://www.merriam-webster.com/dictionary/thence) to HTML). The `polytexnic` gem ships with precompiled Tralics binaries for OS X and select Linux distributions, so that `polytexnic` Just Works™ on most systems, but if your system is not among those supported natively you'll have to compile a version of Tralics yourself. This involves cloning the `tralics` repository, running `make`, and placing the resulting excutable on the path:

    $ git clone https://github.com/softcover/tralics
    $ cd tralics/src
    $ make
    $ cp tralics /usr/local/bin

In the last step, in place of `/usr/local/bin`, you can choose any directory on your path. Depending on your system's permissions, you might also have to run `sudo`:

    $ sudo cp tralics /usr/local/bin

## Contributing a binary

Ideally, we would like the `polytexnic` gem to support as many systems as possible without separate compilation steps. If you needed to compile your own binary and would like to contribute it back to the project, please send an email to <michael@softcover.io> with the `tralics` binary attached and with the result of running the following command:

    $ ruby -e puts 'RUBY_VERSION'

