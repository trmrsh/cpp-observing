/*

Calculate when an object is visible, in between two times, with visible
defined as being less than a certain airmass. Returns false if the object
is never visible
if 

*/

#include "trm_constants.h"
#include "trm_observing.h"

bool Observing::when_visible(const Subs::Position& obj, const Subs::Telescope& telescope, 
			     const Subs::Time& tstart, const Subs::Time& tend, double airmass,
			     Subs::Time& firstvis, Subs::Time& lastvis){

  double altaim   = 90.-360.*acos(1./airmass)/Constants::TWOPI;
  double airstart = obj.altaz(tstart,telescope).alt_true;

  firstvis = tstart;
  if(airstart < altaim && 
     !Observing::startime(obj, telescope, tstart, altaim, firstvis) || firstvis > tend) return false;
   
  lastvis = tend;
  firstvis.add_hour(0.001);
  if(Observing::startime(obj, telescope, firstvis, altaim, lastvis) && lastvis > tend) lastvis = tend;
  firstvis.add_hour(-0.001);
  return true;
}
