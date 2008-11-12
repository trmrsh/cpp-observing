
// Compute when Sun first reaches altitude altaim
// after time = start. If Sun does not do so within
// a day it returns with an error

#include "trm_subs.h"
#include "trm_date.h"
#include "trm_time.h"
#include "trm_position.h"
#include "trm_telescope.h"
#include "trm_observing.h"

bool Observing::suntime(const Subs::Telescope& tel, const Subs::Time& start, double altaim, 
			Subs::Time& found){

  Subs::Position Sun;
  Sun.set_to_sun(start, tel);
  Subs::Altaz altaz = Sun.altaz(start,tel);
  double now  = altaz.alt_true;
  double ha   = altaz.ha;
  double mjd1 = start.mjd();
  double mjd2, crit;

  if(now < altaim){

    // Compute first midday 

    Subs::Time midday = start; 
    if(ha < 0.){
      midday.add_hour(-ha);
    }else{
      midday.add_hour(24.-ha);
    }
    double hi = Sun.altaz(midday,tel).alt_true;
    if(altaim > hi) return false;

    mjd2 = midday.mjd();
    
    while(mjd2-mjd1 > 1.e-5){
      crit = (mjd1+mjd2)/2.;
      found.set(crit);
      Sun.set_to_sun(found, tel);
      if(Sun.altaz(found,tel).alt_true > altaim){
	mjd2 = crit;
      }else{
	mjd1 = crit;
      }
    }
    crit = (mjd1+mjd2)/2.;
    found.set(crit);

  }else{

    // Compute first midnight

    Subs::Time midnight = start; 
    midnight.add_hour(12.-ha);
    double lo = Sun.altaz(midnight,tel).alt_true;
    if(altaim < lo) return false;

    mjd2 = midnight.mjd();
    
    while(mjd2-mjd1 > 1.e-5){
      crit = (mjd1+mjd2)/2.;
      found.set(crit);
      Sun.set_to_sun(found, tel);
      if(Sun.altaz(found,tel).alt_true > altaim){
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
