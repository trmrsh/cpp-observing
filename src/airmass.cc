/*

!!sphinx

airmass plots airmasses versus time.
====================================


**airmass** plots the airmass versus time for a group of stars.Note that the format of the input coordinate/ephemeris files has changed as I
have switched to ICRS coordinates and so no equinix should be specified.

Invocation
----------


airmass file date telescope device

Arguments
---------



  file
    Data file of star position and ephemeris (if applicable)
information. See below for format.}

  date
    Observing date. Calculated as the date on which the
sunset occurs}

  telescope
    Telescope to use. e.g. WHT. Entering a wrong name
will abort the program and dump a list of possibilities.}

  device
    Plot device}

Input file format
-----------------


The star data should be entered as follows:

# is the comment flag 



DQ Her 
        
18 07 30.2 +45 51 32.

HMJD linear 34954.44429  0.00001 0.1936208964 0.0000000001



WD 0101+048

01 03 48.8 +05 04 18.

null



WD 1115+166

11 17 55.4 +16 21 29.

null


The 'null' just indicates that a star has no ephemeris info.  The HMJD above says that the
timescale is heliocentrically-corrected UTC, expressed as an MJD. Other choices are
HJD (heliocentric UTC as a JD), and BJD and BMJD which are the barycentric equivalents using TDB
(no point barycentrically correcting UTC as it is not a good timescale). 'linear' refers to the
number of terms of the ephemeris. The alternative is quadratic in which case a third coefficient
plus an uncertainty would be needed.

!!sphinx

*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "cpgplot.h"
#include "trm/subs.h"
#include "trm/input.h"
#include "trm/plot.h"
#include "trm/date.h"
#include "trm/time.h"
#include "trm/telescope.h"
#include "trm/position.h"
#include "trm/star.h"
#include "trm/binary_star.h"
#include "trm/observing.h"

int main(int argc, char *argv[]){

  try{

    // Construct Input object

    Subs::Input input(argc, argv, Observing::OBSERVING_ENV, Observing::OBSERVING_DIR);

    // sign-in variables (equivalent to ADAM .ifl files)

    input.sign_in("stars",     Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("date",      Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("telescope", Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("device",    Subs::Input::GLOBAL, Subs::Input::PROMPT);

    // Get input

    std::string starfile;
    input.get_value("stars", starfile, "stardata", "file of star positions and ephemerides");

    // Load star data

    std::ifstream file(starfile.c_str());
    if(!file) throw std::string("Could not open file = ") + starfile;

    Subs::Star   s;
    Subs::Binary b;
    Subs::Ephem  eph;
    std::vector<Subs::Star*>  star;
    while(file >> s){
      if(file >> eph){
	star.push_back(new Subs::Binary(s,eph));
      }else{
	if(file.bad()){
	  std::cerr << "File stream corrupted" << std::endl;
	  file.close();
	  exit(EXIT_FAILURE);
	}
	file.clear();
	star.push_back(new Subs::Star(s));
      }
    }
    file.close();
    std::cout << "Found data on " << star.size() << " stars" << std::endl;
    if(star.size() == 0)
      throw std::string("Cannot have 0 stars!");

    std::string sdate;
    input.get_value("date", sdate, "17 Nov 1961", "date at start of night");
    Subs::Date date(sdate);

    std::string stelescope;
    input.get_value("telescope", stelescope, "WHT", "telescope name");
    Subs::Telescope telescope(stelescope);

    std::string device;
    input.get_value("device", device, "/xs", "plot device");

    Subs::Time time(date), sunset, twiend, twistart, sunrise;
    time.add_hour(12.-telescope.longitude()/15.);

    if(!Observing::suntime(telescope, time, -1., sunset))
      throw std::string("Could not find sunset!!");

    if(!Observing::suntime(telescope, sunset, -15., twiend))
      throw std::string("Sun never gets to -15 in evening!!");

    time   = twiend;
    time.add_hour(0.1);   
    if(!Observing::suntime(telescope, time, -15., twistart))
      throw std::string("Sun never gets to -15 in morning!!");

    if(!Observing::suntime(telescope, twistart, -1., sunrise))
      throw std::string("Could not find sunset!!");

    std::cout << "Sunset to sunrise: " << sunset << " to " << sunrise  << std::endl;
    std::cout << "       Sun < -15.: " << twiend << " to " << twistart << std::endl;

    // compute heliocentric corrections with
    // single sun position.

    time.set((sunset.mjd()+sunrise.mjd())/2.);

    Subs::Plot plot(device);
    cpgsch(1.5);
    cpgscf(2);
    cpgsci(4);

    double ut1 = sunset.hour();
    double ut2 = ut1 + 24.*(sunrise.mjd()-sunset.mjd());
    float y1 = 1., y2 = 2.5;
    cpgvstd();
    Subs::ut_plot(ut1,ut2,y1,y2);
    cpgsci(2);
    std::string title = telescope.site() + ", " + date.str();
    cpglab("UT","Airmass",title.c_str());
    cpgsci(1);

    const int NPT=500;
    float x[NPT], y[NPT];
    double ut, mjd0 = floor(sunset.mjd()), mjd;
    double twi1 = 24.*(twiend.mjd()-mjd0);
    double twi2 = 24.*(twistart.mjd()-mjd0);
    int i, n;
    float xt, yt;
    for(size_t j=0; j<star.size(); j++){
      for(i=0, n=0; i<NPT; i++){
	xt = ut = ut1 + (ut2-ut1)*i/(NPT-1);
	mjd = mjd0 + ut/24.;
	time.set(mjd);
	yt = star[j]->altaz(time,telescope).airmass;
	if(yt > 0.5){
	  x[n]   = xt;
	  y[n++] = yt;
	}
      }
      cpgline(n,x,y);
    }

    cpgsci(1);
    cpgsls(2);
    cpgmove(twi1,y1);
    cpgdraw(twi1,y2);
    cpgmove(twi2,y1);
    cpgdraw(twi2,y2);

  }

  catch(const std::string& str){
    std::cerr << str << std::endl;
    exit(EXIT_FAILURE);
  }
  
}






