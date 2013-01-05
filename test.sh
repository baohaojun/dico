#!/bin/bash
if test "$#" = 1 -a "$1" != help; then
    set -- define gcide "$@"
fi
(cat 2.html; echo "$@"| /usr/local/bin/dicod --config /home/bhj/.gcider/dicod.conf -i --stderr |perl -npe 's/^\d{3}.*/<p>/'; echo '</body></html>' ) > 1.html
of 1.html
