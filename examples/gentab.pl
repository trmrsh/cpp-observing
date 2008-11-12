#!/usr/bin/perl
#
# script to generate web pages listing times for VLT run
# arguments:
#
# file1 -- list of times for phase just before eclipse
# file2 -- list of times for phase just after eclipse on same dates
# table -- data file to output

(@ARGV == 3) or die "usage: file1 file2 table\n";

$file1 = shift;
$file2 = shift;
$table = shift;

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

open(TABLE, ">$table");

print TABLE "\n";
print TABLE " Eclipse times for NN Ser, VLT+UVES proposal 073.D-0633\n";
print TABLE " ------------------------------------------------------\n";
print TABLE "\n";

print TABLE "The times listed in the following links show the times of NN Ser's eclipse during which it dims from\n";
print TABLE "V ~ 17 to ~ 23 and would be hard to acquire. The eclipses last only 12 minutes out of the 186 minute orbital\n";
print TABLE "period and so if you are unlucky enough to point at NN Ser during the eclipse, it should not be long\n";
print TABLE "before it appears. The times are based upon a precise ephemeris and are calaculated with corrections for\n";
print TABLE "light-travel time shifts caused by the motion of the Earth. The dates listed apply to the start of the\n";
print TABLE "eclipse in each case\n\n";
print TABLE "NB The times listed are in UT, not local time.\n";

print TABLE "\n--------------------------------------------------\n";
print TABLE " Date at       Start of eclipse    End of eclipse \n";
print TABLE " start of          (UT)                (UT)\n";
print TABLE " eclipse\n";
print TABLE "--------------------------------------------------\n\n";



for $nphase (sort keys %times){
    if(defined $times{$nphase}->{date1} &&
       defined $times{$nphase}->{time1} &&
       defined $times{$nphase}->{time2}){

	$date  = $times{$nphase}->{date1};
	$start = $times{$nphase}->{time1};
	$end   = $times{$nphase}->{time2};
	
	write TABLE;
    }
}

print TABLE "\n\nTom Marsh, Warwick\n";
close(TABLE);


format TABLE =
@<<<<<<<<<<<      @<<<<<<<           @<<<<<<<
$date ,           $start,              $end
.
