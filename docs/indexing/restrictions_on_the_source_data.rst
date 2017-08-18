.. _restrictions_on_the_source_data:

Restrictions on the source data
-------------------------------

There are a few different restrictions imposed on the source data which
is going to be indexed by Manticore, of which the single most important one
is:

**ALL DOCUMENT IDS MUST BE UNIQUE UNSIGNED NON-ZERO INTEGER NUMBERS
(32-BIT OR 64-BIT, DEPENDING ON BUILD TIME SETTINGS).**

If this requirement is not met, different bad things can happen. For
instance, Manticore can crash with an internal assertion while indexing; or
produce strange results when searching due to conflicting IDs. Also, a
1000-pound gorilla might eventually come out of your display and start
throwing barrels at you. You've been warned.
