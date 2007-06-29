//
// $Id$
//

// in this file, one can declare a custom sorting function
// which will be used when sorting by "@custom" in extended sorting mode

// first, declare which attributes will be used in the function
//
// they will be then accessible in the function using MATCH_ATTR(number)
// macro, where "number" is assigned in the order of declaration

MATCH_DECLARE_ATTR ( "group_id" )
MATCH_DECLARE_ATTR ( "date_added" )

// second, define the function itself
//
// in this example, the matches will be sorted by
// @weight + group_id*0.3 - log ( NOW() - date_added )
// in descending order

MATCH_FUNCTION = MATCH_WEIGHT + MATCH_ATTR(0)*0.3f - log ( MATCH_NOW - MATCH_ATTR(1) )

//
// $Id$
//
