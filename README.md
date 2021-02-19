# FSG - Filtered Seed Glitchless

In v 0.5 blind travel filters were added and one nether structure removed.  Bastion in pos/pos, fortress either pos/neg or neg/pos and stronghold within 300 blocks of either 1200,-1200 or -1200,1200 (matching the fortress).

In v 0.4.2 a memory leak was fixed so the code should run faster and crash less

New in v 0.4.1 the bastion in pos/pos is now guaranteed (never basalt)

New in v 0.4: for village spawns the ruined_portals are always above ground, just outside of the village (no further than coords 144, 144), and most importantly the portals always have at least 11 lava. There is a bastion in pos/pos (unless cancelled by basalt) and 1 fortress either pos/neg or neg/pos.

This is a new category for speedrunners designed to capture the feel of RSG with seeds that are of a certain guaranteed quality without the pre-planned feel of SSG.

In essence twitch streamers doing FSG can be more entertaining by finishing more seeds.

Just hit the RUN button to get a never before run seed with good speedrun traits.

The key to this as a category is that the seeds are chosen using a cryptographically secure random number generator and tested after the fact for a few key traits.

So when submitting a run please film the code running that generates your seed and start the world right after.

The current "good seed" traits:
  A village or shipwreck in the pos/pos quadrant of the overworld between 0 and 80 in both X and Z
  A ruined_portal near (0 to 144 but not in the village)
  Your spawn between -48 and 144 in both coordinates
  In the nether there is a structure close to 0,0 in three quadrants (+/+, -/+, +/-) these are all designed to be within 128 of 0,0 in each coordinate although in the negative dimensions they can't be produced with values between -64 and 0.

There will always be 1 or more bastions (pos/pos guaranteed) and exactly 1 fortress generated within 128 blocks of 0,0.

No restrictions are made on the stronghold in anyway.

If you're interested in helping this category grow you can help in several ways:
  Make and record runs and post them in the pb-brag channel, we can keep track of unofficial WRs for this category.
  If you're a seed finder help improve the algorithm, attributes, and the balance between unpredictable and of sufficient quality.
  Help us specify the standards for the category to be serious enough.

Special thanks to all of the Monkeys!