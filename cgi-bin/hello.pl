#!/usr/bin/perl

use strict;
use warnings;
use CGI qw(:standard);

print header(-type => 'text/html', -charset => 'UTF-8');
print start_html('Perl CGI Script');
print h1('Hello, World from Perl!');
print end_html;
