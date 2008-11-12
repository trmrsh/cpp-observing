/*

!!begin
!!title Ephemeris calculations
!!author T.R. Marsh
!!revised 02 Jan 2008
!!root  ephemeris
!!descr prints times corresponding to phases
!!index ephemeris
!!class Programs
!!css   style.css
!!head1 ephemeris -- prints times corresponding to phases

!!head2 Invocation

!!head2 Arguments

!!table
!!arg{file}{Data file of star positions and ephemerides}
!!arg{start}{Date of first night, e.g. 1/5/2002 = 1st May 2002}
!!arg{end}{Date of last night}
!!arg{telescope}{e.g. wht}
!!arg{air}{Maximum airmass to bother with (>1)}
!!arg{sun}{Maximum altitude of sun (in degrees, e.g. -15)}
!!arg{phase}{Phase to report}
!!table

!!end

*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>

#include "trm_subs.h"
#include "trm_constants.h"
#include "trm_input.h"
#include "trm_date.h"
#include "trm_time.h"
#include "trm_telescope.h"
#include "trm_ephem.h"
#include "trm_position.h"
#include "trm_star.h"
#include "trm_binary_star.h"
#include "trm_observing.h"

// Stores star name, orbital phase, airmass and altitude
// of Sun. Used in a map keyed on time.

struct Info{
  std::string name;
  double phase, pherr, airmass, sunalt;
};

int main(int argc, char *argv[]){

  try{

    // Construct Input object

    Subs::Input input(argc, argv, Observing::OBSERVING_ENV, Observing::OBSERVING_DIR);

    // sign-in variables (equivalent to ADAM .ifl files)

    input.sign_in("stars",     Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("startdate", Subs::Input::LOCAL,  Subs::Input::PROMPT);
    input.sign_in("enddate",   Subs::Input::LOCAL,  Subs::Input::PROMPT);
    input.sign_in("telescope", Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("airmass",   Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("sunalt",    Subs::Input::LOCAL,  Subs::Input::PROMPT);
    input.sign_in("phase",     Subs::Input::LOCAL,  Subs::Input::PROMPT);
    input.sign_in("type",      Subs::Input::LOCAL,  Subs::Input::PROMPT);

    // Get input

    std::string starfile;
    input.get_value("stars", starfile, "stardata", "file of star positions and ephemerides");

    // Load star data

    std::ifstream file(starfile.c_str());
    if(!file) throw std::string("Could not open file = ") + starfile;

    Subs::Star   s;
    Subs::Ephem  eph;
    std::vector<Subs::Binary>  binary;
    while(file >> s){
      if(file >> eph){
	binary.push_back(Subs::Binary(s,eph));
      }else{
	if(file.bad()){
	  std::cerr << "File stream corrupted" << std::endl;
	  file.close();
	  exit(EXIT_FAILURE);
	}
	file.clear();
      }
    }
    file.close();
    std::cout << "Found position and ephemeris data on " << binary.size() << " stars" << std::endl;
    if(binary.size() == 0)
      throw std::string("Cannot have 0 stars!");

    std::string sdate;
    input.get_value("startdate", sdate, "17 Nov 1961", "date at start of first night");
    Subs::Date start(sdate);
    input.get_value("enddate", sdate, "17 Nov 1961", "date at start of last night");
    Subs::Date end(sdate);

    if(start > end) throw std::string("Can't have a start date after the end date!");

    std::string stelescope;
    input.get_value("telescope", stelescope, "WHT", "telescope name");
    Subs::Telescope telescope(stelescope);

    double airmass;
    input.get_value("airmass", airmass, 2., 1.001, 50., "maximum airmass to consider");
    double sunalt;
    input.get_value("sunalt", sunalt, -15., -80., 0., "maximum altitude of Sun");
    double phase;
    input.get_value("phase", phase, 0., 0., 1., "orbital phase");

    int nday = int(end.mjd()-start.mjd()+1.5);

    Subs::Time time, sunset, twiend, twistart, sunrise;
    Subs::Date date = start;
    Subs::Position Sun;
    double off[binary.size()];
    double mjd, e1, e2, mjd1, mjd2;
    int ie1, ie2;
    Info info;
    std::map<Subs::Time,Info> times;
    typedef std::map<Subs::Time,Info>::const_iterator CI;

    if(binary.size() == 1){
      std::cout << "\nStar = " << binary[0].name() << ", " << (Subs::Ephem)binary[0] << "\n" << std::endl;
    }else{
      std::cout << "Star                  ";
    }
    std::cout << "  Date            Time           Phase     Error  Airmass  Sun's altitude\n" << std::endl;

    // Now calculate info
    for(int n=0; n<nday; n++){
      time.set(date);
      time.add_hour(12.+telescope.longitude()/15.);

      if(!Observing::suntime(telescope, time, -1., sunset)){
	std::cerr << "Could not find sunset on " << date << std::endl;
	break;
      }
      if(!Observing::suntime(telescope, sunset, sunalt, twiend)){
	std::cerr << "Sun never gets to " << sunalt << " in evening on " << date << std::endl;
	break;
      }
      time   = twiend;
      time.add_hour(0.1);   
      if(!Observing::suntime(telescope, time, sunalt, twistart)){
	std::cerr << "Sun never gets to " << sunalt << " in morning of night starting " 
	     << date << std::endl;
	break;
      }
      if(!Observing::suntime(telescope, twistart, -1., sunrise)){
	std::cerr << "Could not find sunrise in morning of night starting on " <<
	  date << std::endl;
	break;
      }

      // compute corrections with single sun position half-way
      // between sunrise and sunset.

      time.set((sunset.mjd()+sunrise.mjd())/2.);

      for(size_t j=0; j<binary.size(); j++){
	if(binary[j].get_tscale() == Subs::Ephem::BJD || binary[j].get_tscale() == Subs::Ephem::BMJD){
	  off[j] = (time.dtt() + binary[j].tcorr_bar(time,telescope))/Constants::DAY;
	}else if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::HMJD){
	  off[j] = binary[j].tcorr_hel(time,telescope)/Constants::DAY;
	}else{
	  throw std::string("Could not recognize type of timescale for star = " + binary[j].name());
	}
      }

      mjd1 = twiend.mjd();
      mjd2 = twistart.mjd();
      const double MJD2JD = 2400000.5;

      for(size_t nb=0; nb<binary.size(); nb++){
	
	if(binary[nb].get_tscale() == Subs::Ephem::HJD || binary[nb].get_tscale() == Subs::Ephem::BJD){
	  e1 = binary[nb].phase(MJD2JD+mjd1+off[nb]);
	  e2 = binary[nb].phase(MJD2JD+mjd2+off[nb]);
	}else{
	  e1 = binary[nb].phase(mjd1+off[nb]);
	  e2 = binary[nb].phase(mjd2+off[nb]);
	}

	ie1 = int(ceil(e1-phase));
	ie2 = int(floor(e2-phase));
	for(int ie=ie1; ie<=ie2; ie++){
	  if(binary[nb].get_tscale() == Subs::Ephem::HJD || binary[nb].get_tscale() == Subs::Ephem::BJD){
	    mjd  = binary[nb].time(double(ie)+phase)-off[nb]-MJD2JD;
	  }else{
	    mjd  = binary[nb].time(double(ie)+phase)-off[nb];
	  }
	  time.set(mjd);
	  info.name    = binary[nb].name();
	  info.airmass = binary[nb].altaz(time,telescope).airmass;
	  info.phase   = double(ie)+phase;
	  info.pherr   = binary[nb].pherr(binary[nb].time(double(ie)+phase));
	  if(info.airmass > 0.5 && info.airmass < airmass){
	    Sun.set_to_sun(time, telescope);
	    info.sunalt = Sun.altaz(time,telescope).alt_obs;
	    if(info.sunalt < sunalt) times[time]  = info;
	  }
	}

	// Now report results
	for(CI ci=times.begin(); ci != times.end(); ++ci){
	  if(binary.size() > 1){
	    std::cout.setf(std::ios_base::left);
	    std::cout << std::setfill(' ') << std::setw(20) << std::left << ci->second.name << " ";
	  }
	  std::cout << ci->first << " ";
	  std::cout.setf(std::ios_base::left);
	  std::cout << std::setprecision(8) << std::setw(10) << std::setfill(' ') << ci->second.phase << " "
		    << std::setprecision(4) << std::setw(10) << std::setfill(' ') << ci->second.pherr << "   ";
	  std::cout.setf(std::ios_base::left);
	  std::cout << std::setfill(' ') << std::setw(5) << std::setprecision(4)  << ci->second.airmass << "    " 
		    << ci->second.sunalt << std::endl;
	}
	times.clear();
      }
      date.add_day(1);
    }
  }

  catch(const std::string& str){
    std::cerr << str << std::endl;
    exit(EXIT_FAILURE);
  }
  
}




