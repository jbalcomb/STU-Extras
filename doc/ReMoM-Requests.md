


Dratinimaster — 11/2/2024 4:58 AM
I know I wasn't into it, but if you're planning on adding the idea from an earlier discussion, particularly of a racial terrain bonus, I've drawn my own design on the subject (2 or more favored terrains need to exist to adorn the town with the +10% to most lacking resource):

Dwarves: Mountains, adds to power
Barbarians: Mountains, adds to power
High Elves: Forest, adds to food
Dark Elves: Forest, adds to food
Beastmen: Desert, adds to food
Gnolls: Desert, adds to food
Trolls: Swamps, adds to power
Lizardmen: Swamps, adds to power
High Men: Volcanoes, adds to production
Orcs: Volcanoes, adds to production
Halflings: Grasslands, adds to power
Draconians: Mountains, adds to production
Klackons: Swamps, adds to power
Nomads: Desert, adds to production

Further discussion's welcome since it wasn't originally my idea. 
Dratinimaster — 10/30/2024 10:35 AM
Regarding any changes I may want to suggest adding/removing is:
A skill counter for overland map to follow current casting capabilities (preferably in the form of [current/total])
A better(number input-assisted sliders perhaps) method to balance Power distribution into Mana, Research, and Skill
A clickable minimap for navigation
An overland Spell queue like Implode's could be well-received

Dratinimaster — 10/23/2024 5:22 PM
A possibly-catchy thing I discussed before (can't remember where) is the customization of opponents. Sure, being good vs many random opponents makes it seem like you're the best player around. But sometimes one would wish to evaluate their own strategies against  equally-equipped opponents, evaluate the AI itself mid-design, stack more odds against themselves than the random designator can offer, or even execute an exact scenario in search of a certain result/error. 🤔 Who knows, it may have other uses.

Dratinimaster — 10/22/2024 3:52 AM
🤔 For fundamental understanding I offer the following. I hope it helps your decision:
Step#1: Receive input click(Yes. Keyboard won't do with that approach) on a map square.
Step#2: Assign supposed path as per already-implemented pathfinding rules, splitting it to red arrows or other arbitrary motion denoters (Won't suggest boots since they non-thematically appear over water whether sailing or flying) as many as the required steps.
Step#3: Recolor a number of arrows(or denoters)  up to the maximum movement allowance to green.
Step#4: Receiving another click at the same aforementioned square initiates army movement, while receiving it elsewhere reruns steps 1 to 3.
NB: In the case the selected army needed more than a turn to reach their destination, the arrows(denoters) recorded for each group reappear upon next turn start, with more arrows up to this turn's maximum movement allowance for said army turning green. 

Dratinimaster — 10/21/2024 6:49 PM
There was also the part I mentioned before about confirming movement path before execution. Think Heroes of Might and Magic-movement system. It might be a nasty job to incorporate such a thing so I have to leave it up for debate 🤷🏻‍♂️. My reasoning is to avoid movement mistakes that waste movement points. The one solution I know is saving frequently. 
JimBalcomb — 10/21/2024 6:51 PM
I don't know HMM. Is that like switching up the UI and path-finding algo to allow for mousing around to get a prefered movement-path, in real-time?
Maybe like how gMaps allows for a two-stage process of selecting a destination, but adjusting the preferred path?

Dratinimaster — 10/21/2024 7:00 PM
Honestly not sure about gMaps but here's how it went: The turn player clicks a map point/square and a path of disconnected arrows ⤵️ ⬅️ ↖️ is drawn over the map towards said point/square. The number of green arrows starting from the active army's icon is the maximum movement allowance and the rest, if any, are red. If the path won't do for the player they can click again and set another path. If the path is OK, they may click the same point to have the character go through the set path.
Check this video for a visual demo: https://www.youtube.com/watch?v=n7v3N2N7zdE
