/*

!!sphinx

*whatphases* -- plots which phases have been observed
=====================================================

*whatphases* expects a file called <star>.times for each star that has been
observed.

Invocation:
  whatphases stars [device]

Arguments:

  stars :
    Data file of star positions and ephemerides

!!sphinx

*/

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "cpgplot.h"
#include "trm/subs.h"
#include "trm/plot.h"
#include "trm/input.h"
#include "trm/date.h"
#include "trm/time.h"
#include "trm/telescope.h"
#include "trm/ephem.h"
#include "trm/position.h"
#include "trm/star.h"
#include "trm/binary_star.h"
#include "trm/observing.h"

struct Pr{
  Pr() : plo(0.), phi(1.), ci(1), ptype(1) {}
  Pr(float pl, float ph, int col, int pt) : 
    plo(pl), phi(ph), ci(col), ptype(pt) {}
  float plo, phi;
  int ci, ptype;
};

int main(int argc, char *argv[]){

  try{

    // Construct Input object
    Subs::Input input(argc, argv, Observing::OBSERVING_ENV, Observing::OBSERVING_DIR);

    // sign-in variables (equivalent to ADAM .ifl files)
    input.sign_in("stars",  Subs::Input::GLOBAL, Subs::Input::PROMPT);
    input.sign_in("device", Subs::Input::GLOBAL, Subs::Input::PROMPT);

    // Get input values
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
	file.close();
	throw std::string("whatphases only accepts stars with ephemerides");
      }
    }
    file.close();
    std::cout << "Found data on " << binary.size() << " stars" << std::endl;
    if(binary.size() == 0) throw std::string("Cannot have 0 stars!");

    std::string device;
    input.get_value("device", device, "/xs", "plot device");

    // Star data loaded. Now look for files of the form 'star.times'
    // listing the observing times

    Subs::Plot plot(device);

    cpgsch(1.5);
    cpgscf(2);
    cpgsci(4);
    cpgsvp(0.25,0.97,0.15,0.87);
    cpgswin(0.,1.0001,0.,binary.size()+1);
    cpgbox("BCNST",0.,0," ",0.,0);
    cpgsci(2);
    cpglab("Orbital phase"," ","Phase coverage");    
    cpgsci(1);

    // Dummy telescope to allow the barycentric correction to work.
    Subs::Telescope telescope("WHT");

    const double MJD2JD = 2400000.5;
    
    for(size_t nfile=0; nfile<binary.size(); nfile++){
      cpgsci(2);
      float y = float(binary.size()-nfile);
      cpgptxt(-0.02,y,0.,1.,binary[nfile].name().c_str());

      std::string file_of_times = binary[nfile].name() + ".times";
      std::ifstream fstr(file_of_times.c_str());
      cpgsci(1);
      float p1, p2;
      bool plotline = false, poor = false;
      while(fstr){
	std::string line;
	getline(fstr,line);
	if(fstr){
	  plotline = poor = false;
	  if(line.find("Phase ") == 0){

	    std::istringstream istr(line.substr(6));
	    istr >> p1 >> p2;
	    if(!istr) throw std::string("Failed to translate a phase range from ") + file_of_times;
	    plotline = true;

	  }else if(line.find("UT ") == 0){

	    std::string::size_type n = line.find(" to ");
	    if(n == std::string::npos) 
	      throw std::string("Could no find ' to ' in a UT range from ") + file_of_times;
	    std::string stime1 = line.substr(3,n-3); 
	    std::string stime2;
	    std::string::size_type n1 = line.find(" poor");
	    if(n1 == std::string::npos){
	      stime2 = line.substr(n+4);
	    }else{
	      stime2 = line.substr(n+4,n1-n-4);
	      poor = true; // indicate poor conditions
	    }
	    Subs::Time time1(stime1.substr(stime1.find_first_not_of(" \t")));
	    Subs::Time time2(stime2.substr(stime2.find_first_not_of(" \t")));
	    
	    if(time1 > time2) throw std::string("Times out of order in ") + file_of_times;
	    
	    // compute heliocentric corrections with
	    // single sun position.
	    
	    Subs::Time tim;
	    tim.set((time1.mjd()+time2.mjd())/2.);
	    
	    double off;
	    if(binary[nfile].get_tscale() == Subs::Ephem::HJD || binary[nfile].get_tscale() == Subs::Ephem::HMJD){
	      off = binary[nfile].tcorr_hel(tim, telescope)/Constants::DAY;
	    }else if(binary[nfile].get_tscale() == Subs::Ephem::BJD || binary[nfile].get_tscale() == Subs::Ephem::BMJD){
	      off = (tim.dtt() + binary[nfile].tcorr_bar(tim, telescope))/Constants::DAY;
	    }else{
	      throw std::string("Could not recognize type of timescale for star = " + binary[nfile].name());
	    }
	    
	    if(binary[nfile].get_tscale() == Subs::Ephem::HJD || binary[nfile].get_tscale() == Subs::Ephem::BJD){
	      p1 = binary[nfile].phase(MJD2JD+time1.mjd()+off);
	      p2 = binary[nfile].phase(MJD2JD+time2.mjd()+off);
	    }else{
	      p1 = binary[nfile].phase(time1.mjd()+off);
	      p2 = binary[nfile].phase(time2.mjd()+off);
	    }

	    plotline = true;

	  }else if(line.find("JD ") == 0){

	    std::string::size_type n = line.find(" to ");
	    if(n == std::string::npos) 
	      throw std::string("Could no find ' to ' in a JD range from ") + file_of_times;
	    std::string temp;
	    double mjd1, mjd2;
	    std::istringstream istr(line.substr(3));
	    istr >> mjd1 >> temp >> mjd2;
	    mjd1 -= 2400000.5;
	    mjd2 -= 2400000.5;
	    Subs::Time time1(mjd1), time2(mjd2);
	    
	    if(time1 > time2) throw std::string("Times out of order in ") + file_of_times;
	    
	    // Compute heliocentric corrections with
	    // single sun position.
	    
	    Subs::Time tim;
	    tim.set((mjd1+mjd2)/2.);
	    
	    double off;
	    if(binary[nfile].get_tscale() == Subs::Ephem::HJD || binary[nfile].get_tscale() == Subs::Ephem::HMJD){
	      off = binary[nfile].tcorr_hel(tim, telescope)/Constants::DAY;
	    }else if(binary[nfile].get_tscale() == Subs::Ephem::BJD || binary[nfile].get_tscale() == Subs::Ephem::BMJD){
	      off = (tim.dtt() + binary[nfile].tcorr_bar(tim, telescope))/Constants::DAY;
	    }else{
	      throw std::string("Could not recognize type of timescale for star = " + binary[nfile].name());
	    }	    

	    if(binary[nfile].get_tscale() == Subs::Ephem::HJD || binary[nfile].get_tscale() == Subs::Ephem::BJD){
	      p1 = binary[nfile].phase(MJD2JD+time1.mjd()+off);
	      p2 = binary[nfile].phase(MJD2JD+time2.mjd()+off);
	    }else{
	      p1 = binary[nfile].phase(time1.mjd()+off);
	      p2 = binary[nfile].phase(time2.mjd()+off);
	    }
	    plotline = true;

	  }

	  if(plotline){

	    cpgsci(3);
	    // Finally plot
	    int ip1 = int(floor(p1));
	    p1 -= ip1;
	    p2 -= ip1;
	    float tick = 1./4.;
	    cpgsls(1);
	    cpgmove(p1,y-tick);
	    cpgdraw(p1,y+tick);
	    if(poor) cpgsls(2);
	    cpgdraw(p1,y);
	    cpgdraw(std::min(1.f,p2),y);
	    cpgsls(1);
	    cpgmove(std::min(1.f,p2),y-tick);
	    cpgdraw(std::min(1.f,p2),y+tick);
	    if(p2 > 1.){
	      cpgmove(0.f,y-tick);
	      cpgdraw(0.f,y+tick);
	      if(poor) cpgsls(2);
	      cpgmove(0.f,y);
	      cpgdraw(std::min(1.f,p2-1.f),y);
	      cpgsls(1);
	      cpgmove(std::min(1.f,p2-1.f),y-tick);
	      cpgdraw(std::min(1.f,p2-1.f),y+tick);
	    }
	  }
	}
      }
      fstr.close();

      // Over plot the last one in green
      if(plotline){

	cpgsci(3);
	int ip1 = int(floor(p1));
	p1 -= ip1;
	p2 -= ip1;
	float tick = 1./4.;
	cpgsls(1);
	cpgmove(p1,y-tick);
	cpgdraw(p1,y+tick);
	if(poor) cpgsls(2);
	cpgdraw(p1,y);
	cpgdraw(std::min(1.f,p2),y);
	cpgsls(1);
	cpgmove(std::min(1.f,p2),y-tick);
	cpgdraw(std::min(1.f,p2),y+tick);
	if(p2 > 1.){
	  cpgmove(0.f,y-tick);
	  cpgdraw(0.f,y+tick);
	  if(poor) cpgsls(2);
	  cpgmove(0.f,y);
	  cpgdraw(std::min(1.f,p2-1.f),y);
	  cpgsls(1);
	  cpgmove(std::min(1.f,p2-1.f),y-tick);
	  cpgdraw(std::min(1.f,p2-1.f),y+tick);
	}
      }
    }
  }

  catch(const std::string& str){
    std::cerr << "Exception: " << str << std::endl;
    exit(EXIT_FAILURE);
  }
  
}




