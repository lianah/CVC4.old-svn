#!/usr/bin/perl -w
#
# update-copyright.pl
# Morgan Deters <mdeters@cs.nyu.edu> for CVC4
# Copyright (c) 2009-2012  The CVC4 Project
#
# usage: update-copyright [-m] [files/directories...]
#        update-copyright [-h | --help]
#
# This script goes through a source directory rewriting the top bits of
# source files to match a template (inline, below).  For files with no
# top comment, it adds a fresh one.
#
# if no files/directories are unspecified, the script scans its own
# parent directory's "src" directory.  Since it lives in contrib/ in
# the CVC4 source tree, that means src/ in the CVC4 source tree.
#
# If -m is specified as the first argument, all files and directories
# are scanned, but only ones modifed in the current working directory
# are modified (i.e., those that have status M in "svn status").
#
# It ignores any file/directory not starting with [a-zA-Z]
# (so, this includes . and .., vi swaps, .svn meta-info,
# .deps, etc.)
#
# It ignores any file not ending with one of:
#   .c .cc .cpp .C .h .hh .hpp .H .y .yy .ypp .Y .l .ll .lpp .L .g
# (so, this includes emacs ~-backups, CVS detritus, etc.)
#
# It ignores any directory matching $excluded_directories
# (so, you should add here any sources imported but not covered under
# the license.)
#

my $excluded_directories = '^(minisat|bvminisat|cryptominisat|CVS|generated)$';
my $excluded_paths = '^(src/parser/antlr_input_imports.cpp|src/bindings/compat/.*)$';

# Years of copyright for the template.  E.g., the string
# "1985, 1987, 1992, 1997, 2008" or "2006-2009" or whatever.
my $years = '2009-2012';

my $standard_template = <<EOF;
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) $years  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\\endverbatim
EOF

my $public_template = <<EOF;
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) $years  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\\endverbatim
EOF

## end config ##

use strict;
use Fcntl ':mode';

my $dir = $0;
$dir =~ s,/[^/]+/*$,,;

if($#ARGV >= 0 && $ARGV[0] eq '-h' || $ARGV[0] eq '--help') {
  open(my $SELF, $0) || die "error opening $0 for reading";
  while($_ = <$SELF>) {
    last if !/^#/;
    print;
  }
  close $SELF;
  exit;
}

# whether we ONLY process files with svn status "M"
my $modonly = 0;

if($#ARGV >= 0 && $ARGV[0] eq '-m') {
  $modonly = 1;
  shift;
}

my @searchdirs = ();
if($#ARGV == -1) {
  (chdir($dir."/..") && -f "src/include/cvc4_public.h") || die "can't find top-level source directory for CVC4";
  my $pwd = `pwd`; chomp $pwd;

  print <<EOF;
Warning: this script is dangerous.  It will overwrite the header comments in your
source files to match the template in the script, attempting to retain file-specific
comments, but this isn't guaranteed.  You should run this in an svn working directory
and run "svn diff" after to ensure everything was correctly rewritten.

The directories in which to search for and change sources is:
  $pwd/src
  $pwd/test

Continue? y or n:
EOF

  $_ = <STDIN>; chomp;
  die 'aborting operation' if !( $_ eq 'y' || $_ eq 'yes' || $_ eq 'Y' || $_ eq 'YES' );

  $searchdirs[0] = 'src';
  $searchdirs[1] = 'test';
} else {
  @searchdirs = @ARGV;
}

print "Updating sources...\n";

while($#searchdirs >= 0) {
  my $dir = shift @searchdirs;
  my $mode = (stat($dir))[2] || warn "file or directory \`$dir' does not exist!";
  my $is_directory = S_ISDIR($mode);
  if($is_directory) {
    recurse($dir);
  } else {
    if($dir =~ m,^(.*)\/([^/]*)$,) {
      my($dir, $file) = ($1, $2);
      if($dir eq "") {
        $dir = "/";
      }
      handleFile($dir, $file);
    } else {
      handleFile(".", $dir);
    }
  }
}

sub handleFile {
  my ($srcdir, $file) = @_;
  return if !($file =~ /\.(c|cc|cpp|C|h|hh|hpp|H|y|yy|ypp|Y|l|ll|lpp|L|g|java)$/);
  return if ($srcdir.'/'.$file) =~ /$excluded_paths/;
  return if $modonly  &&`svn status "$srcdir/$file" 2>/dev/null` !~ /^M/;
  print "$srcdir/$file...";
  my $infile = $srcdir.'/'.$file;
  my $outfile = $srcdir.'/#'.$file.'.tmp';
  open(my $IN, $infile) || die "error opening $infile for reading";
  open(my $OUT, '>', $outfile) || die "error opening $outfile for writing";
  open(my $AUTHOR, "$dir/get-authors " . $infile . '|');
  my $author = <$AUTHOR>; chomp $author;
  my $major_contributors = <$AUTHOR>; chomp $major_contributors;
  my $minor_contributors = <$AUTHOR>; chomp $minor_contributors;
  close $AUTHOR;
  $_ = <$IN>;
  if(m,^(%{)?/\*(\*| )\*\*\*,) {
    print "updating\n";
    if($file =~ /\.(y|yy|ypp|Y)$/) {
      print $OUT "%{/*******************                                                        */\n";
      print $OUT "/** $file\n";
    } elsif($file =~ /\.g$/) {
      # avoid javadoc-style comment here; antlr complains
      print $OUT "/* *******************                                                        */\n";
      print $OUT "/*! \\file $file\n";
    } else {
      print $OUT "/*********************                                                        */\n";
      print $OUT "/*! \\file $file\n";
    }
    print $OUT " ** \\verbatim\n";
    print $OUT " ** Original author: $author\n";
    print $OUT " ** Major contributors: $major_contributors\n";
    print $OUT " ** Minor contributors (to current version): $minor_contributors\n";
    print $OUT $standard_template;
    print $OUT " **\n";
    while(my $line = <$IN>) {
      last if $line =~ /^ \*\*\s*$/;
      if($line =~ /\*\//) {
        print $OUT " ** [[ Add lengthier description here ]]\n";
        print $OUT " ** \\todo document this file\n";
        print $OUT $line;
        last;
      }
    }
  } else {
    my $line = $_;
    print "adding\n";
    if($file =~ /\.(y|yy|ypp|Y)$/) {
      print $OUT "%{/*******************                                                        */\n";
      print $OUT "/*! \\file $file\n";
    } elsif($file =~ /\.g$/) {
      # avoid javadoc-style comment here; antlr complains
      print $OUT "/* *******************                                                        */\n";
      print $OUT "/*! \\file $file\n";
    } else {
      print $OUT "/*********************                                                        */\n";
      print $OUT "/*! \\file $file\n";
    }
    print $OUT " ** \\verbatim\n";
    print $OUT " ** Original author: $author\n";
    print $OUT " ** Major contributors: $major_contributors\n";
    print $OUT " ** Minor contributors (to current version): $minor_contributors\n";
    print $OUT $standard_template;
    print $OUT " **\n";
    print $OUT " ** \\brief [[ Add one-line brief description here ]]\n";
    print $OUT " **\n";
    print $OUT " ** [[ Add lengthier description here ]]\n";
    print $OUT " ** \\todo document this file\n";
    print $OUT " **/\n\n";
    print $OUT $line;
    if($file =~ /\.(y|yy|ypp|Y)$/) {
      while(my $line = <$IN>) {
        chomp $line;
        if($line =~ '\s*%{(.*)') {
          print $OUT "$1\n";
          last;
        }
        # just in case something's weird with the file ?
        if(!($line =~ '\s*')) {
          print $OUT "$line\n";
          last;
        }
      }
    }
  }
  while(my $line = <$IN>) {
    print $OUT $line;
  }
  close $IN;
  close $OUT;
  rename($outfile, $infile) || die "can't rename working file \`$outfile' to \`$infile'";
}

sub recurse {
  my ($srcdir) = @_;
  print "in dir $srcdir\n";
  opendir(my $DIR, $srcdir);
  while(my $file = readdir $DIR) {
    next if !($file =~ /^[a-zA-Z]/);

    my $mode = (stat($srcdir.'/'.$file))[2];
    my $is_directory = S_ISDIR($mode);
    if($is_directory) {
      next if $file =~ /$excluded_directories/;
      recurse($srcdir.'/'.$file);
    } else {
      handleFile($srcdir, $file);
    }
  }
  closedir $DIR;
}

### Local Variables:
### perl-indent-level: 2
### End:
