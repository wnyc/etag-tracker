etag-session
============

A simple cross platform session_id tracker that uses etags instead of
cookies.


Installation
------------

    git clone git@github.com:wnyc/etag-tracker.git
    cd etag-tracker/server
    make 

Usage
-----

Point your browser or ajax call to `http://example.com/session_id`.
Receive json containing session id.

How does it work?
-----------------

Its pretty dumb actually.  When you make a call to the etag-tracker
software it looks for a `If-None-Match` header.  If there isn't one,
it returns an etag value and a session id.  If there is, it returns a
304 telling the browser to use whatever session_id it previously
retrieved.



