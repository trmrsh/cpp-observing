
// Compute when a celestial object of a given position first 
// reaches altitude altaim after time = start. If it does not do so within
// a day it returns with an error

#include "trm/date.h"
#include "trm/time.h"
#include "trm/position.h"
#include "trm/telescope.h"
#include "trm/observing.h"

bool Observing::startime(const Subs::Position& obj, const Subs::Telescope& tel, 
			 const Subs::Time& start, double altaim, Subs::Time& found){

  Subs::Altaz altaz = obj.altaz(start,tel);
  double now  = altaz.alt_true;
  double ha   = altaz.ha;
  double mjd1 = start.mjd();
  double mjd2, crit;

  if(now < altaim){

    // Compute first upper meridian transit

    Subs::Time transit = start; 
    if(ha < 0.){
      transit.add_hour(-ha);
    }else{
      transit.add_hour(24.-ha);
    }
    double hi = obj.altaz(transit,tel).alt_true;
    if(altaim > hi) return false; // never reaches this altitude

    mjd2 = transit.mjd();
    
    // now binary chop

    while(mjd2-mjd1 > 1.e-5){
      crit = (mjd1+mjd2)/2.;
      found.set(crit);
      if(obj.altaz(found,tel).alt_true > altaim){
	mjd2 = crit;
      }else{
	mjd1 = crit;
      }
    }
    crit = (mjd1+mjd2)/2.;
    found.set(crit);

  }else{

    // Compute first lower meridian transit

    Subs::Time transit = start; 
    transit.add_hour(12.-ha);
    double lo = obj.altaz(transit,tel).alt_true;
    if(altaim < lo) return false; // never gets this low

    mjd2 = transit.mjd();
    
    while(mjd2-mjd1 > 1.e-5){
      crit = (mjd1+mjd2)/2.;
      found.set(crit);
      if(obj.altaz(found,tel).alt_true > altaim){
	mjd1 = crit;
      }else{
	mjd2 = crit;
      }
    }
    crit = (mjd1+mjd2)/2.;
    found.set(crit);
  }
  return true;
}


