$DIR_SOURCESAFE_DB	= '\\\\drop.mtl.ubisoft.org\\vss\\Gameloft\\VSSGL-Dev';
$ENV{"SSDIR"} = $DIR_SOURCESAFE_DB;
$historyFile = "history.txt";
$labelsFile = "labels.txt";
$releaseNotesFile = "ReleaseNotes.txt";

# Get all labels
    unlink $labelsFile;
    execute("ss History \$/Games/Asphalt1.1 -L -O$labelsFile");

    # Find all labels for the project
    my $label = "Latest version";
    my @labels;
    # Set the first label to the current version
    push(@labels, $label);
    open(INPUTLABELS, $labelsFile);
    while ($label = <INPUTLABELS>)
    {
    	if ($label =~ /^Label: /)
    	{
    		$label =~ s/^Label: //;
    		$label =~ s/\"//g;
    		chomp($label);
    		push(@labels, $label);
    	}
    }
    close (INPUTLABELS);

    # Find comments in history for all found labels and create Release Notes
    open(RELEASENOTES, ">$releaseNotesFile");
	my $labelpos = 0;
    foreach $label (@labels)
    {
    	unlink "$historyFile";
    	if ($labelpos == 0)
    	{
    		execute("ss History \$/Games/Asphalt1.1 -R -I-Y -V~L\"$labels[$labelpos+1]\" -O$historyFile");
    	}
    	else
    	{
    		execute("ss History \$/Games/Asphalt1.1 -R -I-Y -VL\"$labels[$labelpos]\"~L\"$labels[$labelpos+1]\" -O$historyFile");
    	}
    	open(INPUTHISTORY, $historyFile);
    	my $line = 'Changes in version: $label\n';
    	my $featuresHeader = "Features added in $label\n";
    	my $bugfixesHeader = "Bugs fixed in $label\n";
    	my $notesHeader = "Important notes in $label\n";
    	my $features = '';
    	my $bugfixes = '';
    	my $notes = '';
    	while ($line = <INPUTHISTORY>)
    	{
			# Check for a feature comment
			if (($line =~ /^Comment: -/) || ($line =~ /^-/))
			{
				$line =~ s/^Comment: //;
				if (index($features,$line) == -1)
				{
					$features = "$features$line";
				}
			}
			# Check for a bug fix comment
			elsif (($line =~ /^Comment: \*/) || ($line =~ /^\* /)|| ($line =~ /^\*\w/))
			{
				$line =~ s/^Comment: //;
				if (index($bugfixes,$line) == -1)
				{
					$bugfixes = "$bugfixes$line";
				}
			}
			elsif (($line =~ /^Comment: \!/) || ($line =~ /^\!/))
			{
				$line =~ s/^Comment: //;
				if (index($notes,$line) == -1)
				{
					$notes = "$notes$line";
				}
			}
    	}
    	close (INPUTHISTORY);

		if ($notes)
    	{
    	 	print RELEASENOTES "$notesHeader\n$notes\n";
    	}
        if ($features )
    	{
    		print RELEASENOTES "$featuresHeader\n$features\n";
    	}
    	if ($bugfixes)
    	{
    	 	print RELEASENOTES "$bugfixesHeader\n$bugfixes\n";
    	}
    	$labelpos = $labelpos + 1;
    	if ($labelpos == (scalar(@labels)-1))
    	{
    		last;
    	}
    }
    close (RELEASENOTES);
    #Clean up
    unlink $labelsFile;
    unlink $historyFile;
   
sub execute()
{
  my $cmd = $_[0];
  print "$cmd\n";
  print `$cmd`;
  my $result = $? >> 8;
  printf "Exited with value %d\n", $result;
  return $result;
}