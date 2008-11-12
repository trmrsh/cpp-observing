#ifndef TRM_OBSERVING
#define TRM_OBSERVING

#include "trm_subs.h"
#include "trm_telescope.h"
#include "trm_time.h"
#include "trm_position.h"

//! Namespace of observing related routines

/** The routines in this namespace are all to do with observing, 
 * giving stuff like the sun rise and set times etc.
 */

namespace Observing {

  //! Default directory for command defaults
  const char OBSERVING_DIR[] = ".observing";

  //! Environment variable for switching directory for command defaults
  const char OBSERVING_ENV[] = "OBSERVING_ENV";

  //! An exception class.

  /** Observing::Observing_Error is the error class for the Observing programs.
   * It is inherited from the standard string class.
   */
  class Observing_Error : public std::string {
  public:

    //! Default constructor
    Observing_Error() : std::string() {}

    //! Constructor storing a message
    Observing_Error(const std::string& err) : std::string(err) {} 
  };

  //! Computes next rise or set time of the Sun
  bool suntime(const Subs::Telescope& tel, const Subs::Time& start, double altaim, 
	       Subs::Time& found);

  //! Computes next rise or set time of a star
  bool startime(const Subs::Position& obj, const Subs::Telescope& tel, const Subs::Time& start, 
		double altaim, Subs::Time& found);
  
  //! Calculates when an object is visible
  bool when_visible(const Subs::Position& obj, const Subs::Telescope& telescope, 
		    const Subs::Time& tstart, const Subs::Time& tend, double airmass,
		    Subs::Time& firstvis, Subs::Time& lastvis);

};

#endif
