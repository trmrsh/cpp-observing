#!/usr/bin/perl
#
# script to generate web pages listing times for VLT run
# arguments:
#
# file1 -- list of times for phase just before eclipse
# file2 -- list of times for phase just after eclipse on same dates
# dir   -- directory to write web pages

(@ARGV == 3) or die "usage: file1 file2 dir\n";

$file1 = shift;
$file2 = shift;
$dir   = shift;

open(FILE, $file1) or die "Failed to open $file1\n";
while(<FILE>){
    if(/^(\d\d ... \d\d\d\d), (\S*) *(\S*)/){
	$date   = $1;
	$time   = $2;
	$phase  = $3;
	$time   =~ s/\.\d*//;
	$nphase = int($phase+0.5);
	$times{$nphase} = {date1 => $date,
			   time1 => $time};
    }
}
close(FILE);

open(FILE, $file2) or die "Failed to open $file2\n";
while(<FILE>){
    if(/^(\d\d ... \d\d\d\d), (\S*) *(\S*)/){
	$date   = $1;
	$time   = $2;
	$phase  = $3;
	$time   =~ s/\.\d*//;
	$nphase = int($phase+0.5);
	$times{$nphase}->{date2} = $date;
	$times{$nphase}->{time2} = $time;
    }
}
close(FILE);

# work out months involved

%expand = (
	   Jan => January,
	   Feb => February,
	   Mar => March,
	   Apr => April,
	   May => May,
	   Jun => June,
	   Jul => July,
	   Aug => August,
	   Sep => September,
	   Oct => October,
	   Nov => November,
	   Dec => December
	   );
 
# Write main file

open(HTML, ">$dir/index.html");

print HTML "<html>\n";
print HTML "<title>NN Ser eclipse times</title>\n";
print HTML "<body>\n";
print HTML "<h1>Eclipse times for NN Ser, VLT+UVES proposal 073.D-0633</h1>\n";
print HTML "<p>\n";

print HTML "The times listed in the following links show the times of NN&nbsp;Ser's eclipse during which it dims from\n";
print HTML "V ~ 17 to ~ 23 and would be hard to acquire. The eclipses last only 12 minutes out of the 186 minute orbital\n";
print HTML "period and so if you are unlucky enough to point at NN&nbsp;Ser during the eclipse, it should not be long\n";
print HTML "before it appears. The times are based upon a precise ephemeris and are calaculated with corrections for\n";
print HTML "light-travel time shifts caused by the motion of the Earth.\n";
print HTML "<p><strong>NB The times listed are <font color=red>UT</font> not local time</strong>.\n";

print HTML "<ol>\n";

for $nphase (sort keys %times){
    if(defined $times{$nphase}->{date1} && defined $times{$nphase}->{date2}){ 
	$nmonth = $times{$nphase}->{date1};
	$year   = $times{$nphase}->{date1};
	$year   =~ s/\d\d ...//;
	$nmonth =~ s/\d\d //;
	$nmonth =~ s/ \d\d\d\d//;
	if(!(defined $month) || ($nmonth ne $month)){
	    if(defined $month){
		print FILE "</ol>\n</body>\n</html>\n";
		close(FILE);
	    }
	    $month  = $nmonth;
	    $lmonth = $expand{$month};

	    print HTML "<li> <a href=\"$lmonth.html\">$lmonth $year</a>\n";

	    open(FILE,">$dir/$lmonth.html");
	    print FILE "<html>\n";
	    print FILE "<title>Eclipse times for NN Ser, $lmonth $year</title>\n";
	    print FILE "<body>\n";
	    print FILE "<h1>Eclipse times for NN Ser, $lmonth $year</h1>\n";
	    print FILE "<p>\nThe times are listed in UT; the dates are correct for the start of eclipse in each case.\n<p>\n<table>\n";
	    print FILE "<tr><th>Date</th> <th>&nbsp;&nbsp;</th><th>Start time</th><th></th><th>End time</th>\n";
	}
	print FILE "<tr><td>$times{$nphase}->{date1}</td><td></td><td>$times{$nphase}->{time1}</td><td>&nbsp;to&nbsp;</td><td>$times{$nphase}->{time2}</td>\n";
    }
}
if(defined $month){
    print FILE "</table>\n</body>\n</html>\n";
    close(FILE);
}
print HTML "</ol>\n<p>\n<address>Tom Marsh, Warwick</address>\n</body>\n</html>\n";
close(HTML);

#for $nphase (sort keys %times){
#    if(defined $times{$nphase}->{date1} && defined $times{$nphase}->{date2}){ 
#	print "
#    }
#}
