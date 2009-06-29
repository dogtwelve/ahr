#!/usr/bin/perl

$regexp = shift;
$subst = shift;
$filenameSource = shift;
$filenameDestination = shift;
#print "Regexp:".$regexp."\n";
#print "Substitute with:".$subst."\n";
#print "Source file:".$filenameSource."\n";
#print "Destination file:".$filenameDestination."\n";

open( FILESOURCE, '<', $filenameSource );

@filestring = <FILESOURCE>;

close( FILESOURCE );

open( FILEDESTIONATION, '>', $filenameDestination );

foreach $row ( @filestring ) {
	$row =~ s/$regexp/$subst/g;
	print FILEDESTIONATION $row;
}
close( FILEDESTIONATION );
