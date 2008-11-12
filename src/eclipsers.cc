/*

!!begin
!!title Eclipse plotter
!!author T.R. Marsh
!!created 2001
!!revised 02 Jan 2008
!!root  eclipsers
!!descr plots critical orbital phases
!!index eclipsers
!!class Programs
!!css   style.css
!!head1 eclipsers -- to plot critical phases

!!emph{eclipsers} give a graphical representation of binary phases. 
It works with a variety of input ephemerides.

!!head2 Invocation

eclipsers file date telescope npoint airmass device

!!head2 Arguments

!!table
!!arg{file}{Data file of star positions and ephemerides}
!!arg{date}{Date of night, e.g. 1/5/2002 = 1st May 2002}
!!arg{telescope}{e.g. wht}
!!arg{npoint}{Number of points to plot across night}
!!arg{airmass}{Maximum airmass to allow}
!!table

!!end

*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "cpgplot.h"
#include "trm_subs.h"
#include "trm_constants.h"
#include "trm_input.h"
#include "trm_plot.h"
#include "trm_date.h"
#include "trm_time.h"
#include "trm_telescope.h"
#include "trm_ephem.h"
#include "trm_position.h"
#include "trm_star.h"
#include "trm_binary_star.h"
#include "trm_observing.h"

struct Pr{
  Pr() : plo(0.), phi(1.), ci(1), lw(1) {}
  Pr(float pl, float ph, int col, int lwid) : 
    plo(pl), phi(ph), ci(col), lw(lwid) {}
  float plo, phi;
  int ci, lw;
};

int main(int argc, char *argv[]){

  // The phase ranges and colour indices to use for them

  const int NRANGE = 2;
  Pr pr[NRANGE];
  pr[0] = Pr(0.95, 1.05, 3, 12);
  pr[1] = Pr(0.45, 0.55, 2, 6);

  try{

    // Construct Input object
    Subs::Input input(argc, argv, Observing::OBSERVING_ENV, Observing::OBSERVING_DIR);

    // sign-in variables (equivalent to ADAM .ifl files)
    input.sign_in("stars",     Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("date",      Subs::Input::GLOBAL,  Subs::Input::PROMPT);
    input.sign_in("telescope", Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("airmass",   Subs::Input::LOCAL,  Subs::Input::PROMPT);
    input.sign_in("device",    Subs::Input::GLOBAL, Subs::Input::PROMPT);

    // Get input
    std::string starfile;
    input.get_value("stars", starfile, "stardata", "file of star positions and ephemerides");

    // Load star data
    std::ifstream file(starfile.c_str());
    if(!file) throw std::string("Could not open file = ") + starfile;

    Subs::Star   star;
    Subs::Ephem  eph;
    std::vector<Subs::Binary>  binary;
    while(file >> star){
      if(file >> eph){
	binary.push_back(Subs::Binary( star, eph));
      }else{
	if(file.bad()){
	  file.close();
	  throw std::string("File stream corrupted");
	}
	file.clear();
      }
    }
    file.close();
    std::cout << "Found position and ephemeris data on " << binary.size() << " stars " << std::endl;
    if(binary.size() == 0)
      throw std::string("Cannot have 0 stars!");

    std::string sdate;
    input.get_value("date", sdate, "17 Nov 1961", "date at start of night");
    Subs::Date date(sdate);

    std::string stelescope;
    input.get_value("telescope", stelescope, "WHT", "telescope");
    Subs::Telescope telescope(stelescope);

    float airmass;
    input.get_value("airmass", airmass, 2.f, 1.001f, 50.f, "maximum airmass");

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

    Subs::Plot plot(device);

    cpgsch(1.5);
    cpgslw(2);
    cpgscf(2);
    cpgsci(4);

    double ut1 = sunset.hour(), ut2 = ut1 + 24.*(sunrise.mjd()-sunset.mjd());
    cpgsvp(0.24,0.96,0.15,0.87);
    Subs::ut_plot(ut1, ut2, 0., binary.size()+3+NRANGE, false);
    cpgsci(2);
    cpglab("UT"," ",date.str().c_str());

    cpgsch(1.);
    cpgptxt(ut1, 1.03*(binary.size()+3+NRANGE), 0., 0.5,"sunset");
    cpgptxt(ut2, 1.03*(binary.size()+3+NRANGE), 0., 0.5,"sunrise");
    cpgsch(1.5);

    const double MJD2JD = 2400000.5;

    double mjd0 = floor(sunset.mjd());
    double twi1 = 24.*(twiend.mjd()-mjd0);
    double twi2 = 24.*(twistart.mjd()-mjd0);
    float x, y;

    for(size_t j=0; j<binary.size(); j++){

      // Print name
      cpgsci(2);
      cpgsch(1.2);
      x = ut1-(ut2-ut1)/30.;
      y = binary.size() - j;
      cpgslw(1);
      cpgptxt(x,y,0.,1.,binary[j].name().c_str());

      // Times when object is visible, limited by sunrise and set
      Subs::Time tfirst, tlast;
      if(Observing::when_visible(binary[j], telescope, sunset, sunrise, airmass, tfirst, tlast)){

	double t1 = 24.*(tfirst.mjd()-mjd0), t2 = 24.*(tlast.mjd()-mjd0);
	time.set((sunset.mjd()+sunrise.mjd())/2.);

	// Plot dashed line for visible period
	cpgsci(1);
	cpgsls(2);
	cpgslw(1);
	cpgmove(t1,y);
	cpgdraw(t2,y);
	
	double off, offm; 
	if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::HMJD){
	  offm = off = binary[j].tcorr_hel(tfirst, telescope)/Constants::DAY;
	}else if(binary[j].get_tscale() == Subs::Ephem::BJD || binary[j].get_tscale() == Subs::Ephem::BMJD){
	  offm = off = (tfirst.dtt() + binary[j].tcorr_bar(tfirst, telescope))/Constants::DAY;
	}else{
	  throw std::string("Could not recognize type of timescale for star = " + binary[j].name());
	}

	double time = tfirst.mjd() + off;
	if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::BJD)
	  time += MJD2JD;

	double phase1 = binary[j].phase(time);

	if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::HMJD){
	  off = binary[j].tcorr_hel(tlast, telescope)/Constants::DAY;
	}else if(binary[j].get_tscale() == Subs::Ephem::BJD || binary[j].get_tscale() == Subs::Ephem::BMJD){
	  off = (tlast.dtt() + binary[j].tcorr_bar(tlast, telescope))/Constants::DAY;
	}

	time = tlast.mjd() + off;
	if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::BJD)
	  time += MJD2JD;
	double phase2 = binary[j].phase(time);

	offm += off;
	offm /= 2.;

	double p1, p2;
	cpgsls(1);
	for(int i=0; i<NRANGE; i++){
	  double ip = floor(phase1);
	  while((p1 = ip+pr[i].plo) < phase2){
	    p1 = std::max(phase1, p1);
	    p2 = std::min(phase2, ip+pr[i].phi);
	    if(p1 < p2){
	      if(binary[j].get_tscale() == Subs::Ephem::HJD || binary[j].get_tscale() == Subs::Ephem::BJD){
		t1 = 24.*(binary[j].time(p1)-offm-mjd0-MJD2JD);
		t2 = 24.*(binary[j].time(p2)-offm-mjd0-MJD2JD);
	      }else{
		t1 = 24.*(binary[j].time(p1)-offm-mjd0);
		t2 = 24.*(binary[j].time(p2)-offm-mjd0);
	      }

	      cpgsci(pr[i].ci);
	      cpgslw(pr[i].lw);
	      cpgmove(t1, y);
	      cpgdraw(t2, y);

	    }
	    ip++;
	  }
	}
      }else{
	std::cout << binary[j].name() << " is never below airmass = " << airmass << " from sunset to sunrise." << std::endl;
      }
    }
    
    // Plot key
    cpgsch(1.4);
    x = ut1;
    for(int j=0; j<NRANGE; j++){
      std::string label = "Phase range: " + Subs::str(pr[j].plo) + " to " + Subs::str(pr[j].phi);
      y = binary.size()+NRANGE-j+1;
      cpgsci(pr[j].ci);
      cpgslw(pr[j].lw);
      cpgmove(x,y);
      cpgdraw(x+(ut2-ut1)/20.,y);
      cpgslw(1);
      cpgptxt(x+(ut2-ut1)/15.,y-0.015,0.,0.,label.c_str());
    }

    // Plot dashed lines for when Sun is at -15
    cpgsci(1);
    cpgsls(2);
    cpgmove(twi1,0.);
    cpgdraw(twi1,1.05*binary.size());
    cpgmove(twi2,0.);
    cpgdraw(twi2,1.05*binary.size());

    // Plot dotted lines on hour boundaries
    cpgsls(4);
    for(int j=int(ceil(twi1)); j<int(ceil(twi2)); j++){
      cpgmove(j,0.);
      cpgdraw(j,1.05*binary.size());
    }
  }

  catch(const std::string& str){
    std::cerr << str << std::endl;
    exit(EXIT_FAILURE);
  }
  
}




