This project revamps an old physics engine I stopped actively working on in February 2026. I got distracted by Vulkan and updating index_set before I could implement many features. This is me going back to finish what I started.

This engine has: Fully working graph coloring (allowing Gauss-Seidel to be paralyzed). Polygon and circle colliders. GJK for poly-poly, SAT for poly-circle, and radius check for circle-circle. Three coordinate types: rigid body, particle, and liquid particle. Temporary contacts, Persistent contacts, and Pin constraints.
	
The main goal of this engine is compile-time flexibility. You can completely add/remove entire coordinate types, modify the interactions between them, and change the systems that act on them with little effort.

This is achieved primarily through the god::state class and the index_set library.

The god::state class defines a tuple where all elements can access all the information in the other elements, including at compile time. For example, say I have a tuple of employees {boss, employment_advisor, environment_advisor, some_guy}. The boss type could look in its scope, find both advisor types with a trait, store their phone numbers in an array (compile-time size), and then when boss.make_decision() is called, the boss can call both advisors and make a decision.

The index_set library allows data to track indices. For example, if I have an indexed set of customers, 'c', I could make a name vector that tracks 'c' in one place and a birth date vector that tracks 'c' in another, wherever is best. It also works for multi-rank sets. Say I separate each customer vector by region; then I would make the new set indexed by \[reg\]\[c\], and it too can be tracked by data.

I want to note that the version of index_set that this physics engine uses is incredibly outdated. I had a breakthrough at the hospital that makes the interface about a billion times easier to use, but I can't remake the entire engine in the time I have, so I decided to make do with the old version of index_set.

![](https://github.com/Nelthy/maker/blob/main/Maker%20Pyramid%20Fallapart.gif)
