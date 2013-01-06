/*

!!begin
!!title  Starinfo
!!author T.R. Marsh
!!created 27 July 2001
!!revised 02 Jan 2008
!!root  starinfo
!!descr real time information on airmasses etc
!!index starinfo
!!class Programs
!!css   style.css
!!head1 starinfo for real time airmass etc information.

!!emph{starinfo} prints out useful information on a star in real time and,
optionally, somewhat ahead of real time. This allows one to judge when to change objects
to catch particular phases etc. For each target it prints the hour angle, airmass and 
the position angle for a vertical slit. For targets with ephemerides, it also prints
the orbital phase and uncertainty.

!!head2 Arguments
!!table
!!arg{stars}{Data file on stars of interest}
!!arg{telesope}{Telescope name. A wrong entry will produce listing}
!!arg{advance}{Number of hours to look ahead. advance = 0 will mean only one time is computed.}
!!arg{present}{Use present time (from computer) or not}
!!arg{time}{If present = false, then this is the time that will be used. String of the form:
"11 May 2032, 15:03:34.22" (exactly so, including the quotes).}
!!table

!!end

*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <vector>

#include "trm_subs.h"
#include "trm_constants.h"
#include "trm_date.h"
#include "trm_time.h"
#include "trm_telescope.h"
#include "trm_position.h"
#include "trm_star.h"
#include "trm_binary_star.h"
#include "trm_input.h"
#include "trm_observing.h"

int main(int argc, char *argv[]){

  try{

    // Construct Input object

    Subs::Input input(argc, argv, Observing::OBSERVING_ENV, Observing::OBSERVING_DIR);

    // sign-in variables (equivalent to ADAM .ifl files)

    input.sign_in("stars", Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("telescope", Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("advance", Subs::Input::LOCAL, Subs::Input::PROMPT);
    input.sign_in("present", Subs::Input::LOCAL, Subs::Input::PROMPT);
    input.sign_in("time", Subs::Input::LOCAL, Subs::Input::PROMPT);

    // Get input

    std::string starfile;
    input.get_value("stars", starfile, "stardata", "file of star positions and ephemerides");

    // Load star data

    std::ifstream file(starfile.c_str());
    if(!file) throw std::string("Could not open file = ") + starfile;

    std::vector<Subs::Star*>  star;
    Subs::Star   s;
    Subs::Binary b;
    Subs::Ephem  eph;
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
    if(star.size() == 0) throw std::string("Cannot have 0 stars!");

    size_t lmax = 0;
    for(size_t j=0; j<star.size(); j++) lmax = std::max(lmax,star[j]->name().length());

    std::string stelescope;
    input.get_value("telescope", stelescope, "WHT", "telescope in question");

    Subs::Telescope telescope(stelescope);

    double advance;
    input.get_value("advance", advance, 1., -12., 12., "number of hours in advance of current time");

    bool present;
    input.get_value("present", present, true, "use present time as first time?");

    std::string stime;
    Subs::Time time;
    if(!present){
      input.get_value("time", stime, "17 Nov 1961, 01:23:45.67", "time (overrides present time)");
      time.set(stime);
    }

    const double MJD2JD = 2400000.5;
    char c = 'm';
    Subs::Time t;
    Subs::Altaz a;
    std::string str;
    double off, ha, pa;
    Subs::Position Sun;
    Subs::Altaz saltaz;
    while(c != 'q' && c != 'Q'){
      if(present){
	t.set();
      }else{
	t = time;
      }

      Sun.set_to_sun(t, telescope);
      saltaz = Sun.altaz(t,telescope);

      std::cout << "\n\n\n" << t << ", MJD = " << std::setprecision(10) << t.mjd() 
		<< ", Sun's altitude = " << std::setprecision(4) << saltaz.alt_true << "\n" << std::endl;

      for(size_t j=0; j<star.size(); j++){
	a   = star[j]->altaz(t,telescope);

	ha  = floor(100.*a.ha+0.5)/100.;
	pa  = floor(100.*a.pa+0.5)/100.;
	std::cout << std::setfill(' ') << std::setw(lmax) << std::left << star[j]->name() << " " 
		  << " HA = " << std::setprecision(3) << std::setw(6) << ha 
		  << ", airmass = " << std::setprecision(3) << std::setw(4) << a.airmass
		  << ", PA = " << std::setprecision(4) << std::setw(5) << pa
		  << ", azimuth = " << std::setprecision(4) << std::setw(5) << a.az;


	// Phase information

	if(star[j]->has_ephem()){

	  // adjust offset according to the timescale of the ephemeris
	  if(star[j]->get_tscale() == Subs::Ephem::HJD || star[j]->get_tscale() == Subs::Ephem::HMJD){
	    off = star[j]->tcorr_hel(t, telescope)/Constants::DAY;
	  }else if(star[j]->get_tscale() == Subs::Ephem::BJD || star[j]->get_tscale() == Subs::Ephem::BMJD){
	    off = (t.dtt() + star[j]->tcorr_bar(t, telescope))/Constants::DAY;
	  }else{
	    throw std::string("Could not recognize type of timescale for star = " + star[j]->name());
	  }

	  if(star[j]->get_tscale() == Subs::Ephem::HJD || star[j]->get_tscale() == Subs::Ephem::BJD){
	    std::cout << ", phase = " << std::setprecision(10) 
		 << star[j]->phase(MJD2JD+t.mjd()+off) << ", error = " << std::setprecision(5)  
		 << star[j]->pherr(MJD2JD+t.mjd()+off) << std::endl;
	  }else{
	    std::cout << ", phase = " << std::setprecision(10) 
		 << star[j]->phase(t.mjd()+off) << ", error = " << std::setprecision(5)  
		 << star[j]->pherr(t.mjd()+off) << std::endl;
	  }

	}else{
	  std::cout << std::endl;
	}
      }
      if(advance != 0.){

	t.add_hour(advance);
	Sun.set_to_sun(t, telescope);
	saltaz = Sun.altaz(t,telescope);

	std::cout << "\nand in " << advance << " hours time, Sun's altitude = " << saltaz.alt_true 
		  << " and:\n" << std::endl;

	for(size_t j=0; j<star.size(); j++){
	  a   = star[j]->altaz(t,telescope);
	  
	  ha  = floor(100.*a.ha+0.5)/100.;
	  pa  = floor(100.*a.pa+0.5)/100.;
	  std::cout << std::setfill(' ') << std::setw(lmax) << std::left << star[j]->name() 
		    << " " << " HA = " << std::setprecision(3) << std::setw(6) << ha 
		    << ", airmass = " << std::setprecision(3) << std::setw(4) << a.airmass
		    << ", PA = " << std::setprecision(4) << std::setw(5) << pa
		    << ", azimuth = " << std::setprecision(4) << std::setw(5) << a.az;
	  
	  if(star[j]->has_ephem()){

	    // adjust offset according to the timescale of the ephemeris
	    if(star[j]->get_tscale() == Subs::Ephem::HJD || star[j]->get_tscale() == Subs::Ephem::HMJD){
	      off = star[j]->tcorr_hel(t, telescope)/Constants::DAY;
	    }else if(star[j]->get_tscale() == Subs::Ephem::BJD || star[j]->get_tscale() == Subs::Ephem::BMJD){
	      off = (t.dtt() + star[j]->tcorr_bar(t, telescope))/Constants::DAY;
	    }else{
	      throw std::string("Could not recognize type of timescale for star = " + star[j]->name());
	    }
	    
	    if(star[j]->get_tscale() == Subs::Ephem::HJD || star[j]->get_tscale() == Subs::Ephem::BJD){
	      std::cout << ", phase = " << std::setprecision(10) 
		   << star[j]->phase(MJD2JD+t.mjd()+off) << ", error = " << std::setprecision(5)  
		   << star[j]->pherr(MJD2JD+t.mjd()+off) << std::endl;
	    }else{
	      std::cout << ", phase = " << std::setprecision(10) 
		   << star[j]->phase(t.mjd()+off) << ", error = " << std::setprecision(5)  
		   << star[j]->pherr(t.mjd()+off) << std::endl;
	    }
	    
	  }else{
	    std::cout << std::endl;
	  }
	}
      }
      if(present){
	std::cout << "\nQ(uit), anything else to continue: ";
	std::cin.get(c);
      }else{
	c = 'q';
      }
    }
  }

  catch(const std::string& str){
    std::cerr << str << std::endl;
    exit(EXIT_FAILURE);
  }
  
}




