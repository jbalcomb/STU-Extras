
https://masterofmagic.fandom.com/wiki/Player_to_AI_Diplomacy

Player to AI Diplomacy

Contents
1	Player to AI diplomacy
1.1	Initial check
1.2	End of audience
1.3	
2	Available options during the audience
3	Propose Treaty
3.1	Wizard's Pact
3.2	Alliance
3.3	Peace Treaty
3.4	Declare War on
3.4.1	Known bugs
3.4.2	Decision process for 1.40 and below.
3.4.3	Decision process for the 1.50 unofficial patch.
3.5	Break Alliance with
3.5.1	Decision process for 1.40 and below.
3.5.2	Decision process for the 1.50 unofficial patch.
4	Threaten / Break Treaty
4.1	Break Wizard's Pact
4.2	Break Alliance
4.3	Threaten to attack
4.3.1	Decision process for 1.40 and below.
4.3.2	Decision process for the 1.50 unofficial patch.
5	Offer Tribute
6	Exchange Spells

Player to AI diplomacy

As soon as Diplomatic Contact has been estabilished with another wizard, the human player can click on the portrait gem of that wizard to request an audience.
Upon doing so, an initial check is done to see if the wizard is willing to listen to the player, and a greeting text is show.
Afterwards, if the wizard is willing to talk, the player is presented a menu system containing possible options. However, if at any time, the wizard grows impatient, they will terminate the audience.
There is an exception to this rule: If a Diplomatic Reaction involving that wizard happened this turn, and queued up a Diplomatic Message, that message will be shown instead in advance, prior to the AI's turn when it might or might not be repeated, but in either case, no audience can be started with that wizard that turn. The AI wizard is too focused on the mentioned topic to discuss anything else. As such, it is always recommended to request an audience before casting spells or moving troops that might trigger a message.

Initial check
Immediately after clicking on the wizard's portrait, the game selects the lowest of Treaty Interest, Peace Interest, and Trade Interest. Then calculates the following:

X= Personality Modifier + Lowest Interest + Hidden Relation + Visible Relation - Visible Relation

(Visible Relation is both added and subtracted, so has no actual effect in this part)

Afterwards, if X<=-100, the wizard refuses to meet with the player.

If Personality Modifier + Lowest Interest + Hidden Relation + Visible Relation is between -99 and -50, they will use an impatient greeting, if above -50 then a normal one.

The 1.50 unofficial patch simplifies these checks to be Lowest interest<=-100 and Lowest interest<=-50 respectively. This change makes it consistent with the audience ending condition, and prevents audiences that start but end before the player can take any action. Additionally, Peace Interest is ignored when determining the lowest interest variable, as it is functionally different now.

End of audience 
The player may end the audience any time, by selecting the "Good Bye" option from the menu.

The AI wizard will immediately end the audience when the lowest of Treaty Interest, Peace Interest, and Trade Interest is below -100. This is inconsistent with the check for allowing the audience, and in most cases, less forgiving, so it is possible for the audience to start, and immediately end before the player can do anything.

Available options during the audience
You have 5 options in the main menu. You can propose a treaty, perform a hostile action (break a treaty or threaten with a war), trade spells, or offer a tribute. You can also opt for ending the audience and returning to the magic screen.

## Propose Treaty
Entering or leaving this submenu has no hidden effect, and only serves as a step towards selecting the following options :

### Wizard's Pact
When selecting this option, there are many possible outcomes :
-The AI refuses the offer without considering it.
-The AI considers the offer but refuses anyway.
-The AI accepts the offer but demands gold, or a spell in exchange.
-The AI accepts the offer for free.
If the proposal was accepted, a Wizard's Pact is formed between the player and the AI wizard. For more details on the decision process, see the Wizard's Pact page.

### Alliance
The possible outcomes are identical to the Wizard's Pact option. If the proposal was accepted, an Alliance is formed instead. See the decision process on the Alliance page.

### Peace Treaty
When the player proposes a Peace Treaty, the AI will always consider the offer, but might refuse, demand additional gold, a spell, or accept like usual.
This option is only available if you are at War with the wizard already.
If the proposal is accepted, the War treaty status is changed to None, and the Peace Counter is set to a random amount. See Peace Treaty for details.
Regardless of the outcome, Treaty Interest, Peace Interest and Trade Interest are all decreased by 20 each time this option was selected.

### Declare War on
When the player has an Alliance with the wizard he is proposing this to, and are at War with the target, the AI wizard is required to accept the proposal. However, there is a 50% chance the AI wizard will demand gold or a spell in exchange for the request. If the player does not have any valid spell, and does not have the amount of gold required, the AI will refuse the proposal without mentioning this being the reason.

In any other case, the AI will consider the offer, and might refuse, demand additional gold, a spell, or accept it.

If the proposal is accepted, the AI wizard declares War on the target immediately.

Known bugs
If the wizard has a Wizard's Pact or Alliance with the target, they are supposed to reduce the chance of the AI accepting the offer, but they do not. The unofficial 1.40n patch fixes this problem. Additionally, the unofficial 1.50 patch disables this menu item if an Alliance is present, in which case, "Break Alliance with" has to be used successfully first.

Decision process for 1.40 and below.
First, the Proposal Difficulty of the request is calculated.

Proposal Difficulty = 150 + the Visible Relation between the two AI wizards + (30 if they have a Wizard's Pact) + (60 if they have an Alliance)

Afterwards the following formula is evaluated (all variables are for the relation between the player and the AI) :

X= Treaty Interest + Hidden Relation + Visible Relation + Personality Modifier + Military Modifier + (40 if an Alliance is present) - Proposal Difficulty + random (1-100).

If X< -75 then the offer is refused.

If X<-50 but >=-75 then they'll demand a spell.

If X>=-50 but <0 then they'll demand gold.

If X>=0 then they'll accept the proposal.

Finally, the Peace Interest and Trade Interest variables are reduced by 10, and the Treaty Interest variable is reduced by a random amount between 30-60.

Decision process for the 1.50 unofficial patch.
First, the Proposal Difficulty is calculated the same way as above. Then, ((target military power*100/own military power)-75) is added to the proposal difficulty to account for the forces of the two AI wizards involved. Finally, 2* Personality Modifier is added to ensure it has a reversed effect than usual : a more peaceful wizard is less likely to accept this type of proposal.

Afterwards, X is calculated with a new formula :

X= Treaty Interest /2 + Hidden Relation + Visible Relation + Personality Modifier + Military Modifier + (40 if an Alliance is present) - Proposal Difficulty - 10*Game Difficulty + random(1-30) ,

where Game Difficulty is 0 (Easy) to 4 (Impossible), and Military Modifier uses a new calculation, see that page for details.

Finally, based on the value of X, the offer will be accepted, refused or additional demands are made, the same way as in previous versions.

### Break Alliance with
The possible outcomes are identical to the Wizard's Pact option. If the proposal is accepted, the AI wizard will perform a Break Treaty action on the target wizard.

Decision process for 1.40 and below.
First, the Proposal Difficulty of the request is calculated.

Proposal Difficulty = 175 + the Visible Relation between the two AI wizards.

Afterwards the following formula is evaluated (all variables are for the relation between the player and the AI) :

X= Treaty Interest + Hidden Relation + Visible Relation + Personality Modifier + Military Modifier + (40 if an Alliance is present) - Proposal Difficulty + random (1-100).

If X< -75 then the offer is refused.

If X<-50 but >=-75 then they'll demand a spell.

If X>=-50 but <0 then they'll demand gold.

If X>=0 then they'll accept the proposal.

Finally, the Peace Interest and Trade Interest variables are reduced by 10, and the Treaty Interest variable is reduced by a random amount between 30-60.

Decision process for the 1.50 unofficial patch.
First, the Proposal Difficulty is calculated the same way as above. Then, ((target military power*100/own military power)-75) is added to the proposal difficulty to account for the forces of the two AI wizards involved. Finally, 2* Personality Modifier is added to ensure it has a reversed effect than usual : a more peaceful wizard is less likely to accept this type of proposal.

Afterwards, X is calculated with a new formula :

X= Treaty Interest /2 + Hidden Relation + Visible Relation + Personality Modifier + Military Modifier + (40 if an Alliance is present) - Proposal Difficulty - 10*Game Difficulty + random(1-30) ,

where Game Difficulty is 0 (Easy) to 4 (Impossible), and Military Modifier uses a new calculation, see that page for details.

Finally, based on the value of X, the offer will be accepted, refused or additional demands are made, the same way as in previous versions.

## Threaten / Break Treaty
Entering this menu has no hidden effect, but leaving it for any reason immediately results in a penalty of 100 to Peace Interest, Treaty Interest, and Trade Interest. This includes leaving the screen without doing anything, through selecting the "Forget it" option, as well as selecting any option that actually does something. This is in addition to any changes that happen to these variables as a result of selecting those options.

It is unknown if this was intended, or the effect applying if nothing is done is a bug. However, in the unofficial 1.50 patch, the variable reduction no longer happens if "Forget It" is selected.

Break Wizard's Pact
Selecting this option results in a Break Treaty action between you and the AI wizard.

Break Alliance
Selecting this option results in a Break Treaty action between you and the AI wizard.

Threaten to attack
Selecting this option can result in diverse replies from the other wizard :
-They might ignore the threat and do nothing
-They might offer you gold as a tribute
-They might offer you a spell as a tribute
-They might consider this a formal declaration of war from your side.

Decision process for 1.40 and below.
First calculate
X = Military Modifier + Hidden Relation + Personality Modifier + random(1-100) + random(1-100)
The Visible Relation has no effect on the outcome!
If X<0, the act counts as the player declaring War on the wizard immediately.
If 0<=X<+90, the threats will be ignored.
If X>=90 but <+200, the AI will offer gold to the player. The offered amount will be (random(8)+2)*Current Turn rounded down to the nearest 25. If the AI has less gold, the amount will be 1/4 of all the gold they have. If they have exactly 0 gold, the threats will be ignored. The player will then receive this amount of gold.
IF X>=+200, the AI will offer a spell if one is available. If none are available, the threats will be ignored.
Regardless of the outcome, the Warning Counter increases by one. The Visible Relation decreases by a random amount between 1-15. Treaty Interest is set to -120. If the AI offered anything, the Peace Counter for the AI is set to 5-20, so the AI will avoid hostile actions which would make their offer pointless. Finally, a random amount of 1-5 is subtracted from the stored diplomatic reaction's strength, regardless of one existing or not.

Known bugs
If the AI wants to offer a spell, but fails to find one, it does not try to offer gold instead, even though that requires a lower total X. So being too successful at threatening a wizard who does not share a realm with the player might fail.
The gold the AI offered is not subtracted from their gold reserve, although the player gains it as expected.
The unofficial 1.50 patch fixes both of these bugs.

Decision process for the 1.50 unofficial patch.
First calculate
X = Military Modifier + Hidden Relation + Personality Modifier + random(1-50) + random(1-50) -10* Game Difficulty.
where Game Difficulty is 0 (Easy) to 4 (Impossible), and Military Modifier uses a new calculation, see that page for details.
The Visible Relation has no effect on the outcome!
If X<0, the act counts as the player declaring War on the wizard immediately.
If 0<=X<+75, the threats will be ignored.
If X>=75 but <+180, the AI will offer gold to the player. The offered amount will be (random(8)+2)*Current Turn rounded down to the nearest 25. If the AI has less gold, the amount will be 1/4 of all the gold they have. If they have exactly 0 gold, the threats will be ignored. The player will then receive this amount of gold. The Peace Counter for the AI is set to 5-20.
IF X>=+180, the AI will offer a spell if one is available. If none are available, do the previous option and offer gold is possible.The Peace Counter for the AI is set to 15-30 if a spell was successfully given.
Regardless of the outcome, the Warning Counter increases by one. The Visible Relation decreases by a random amount between 1-15. Treaty Interest is set to -120. Finally, a random amount of 1-5 is subtracted from the stored diplomatic reaction's strength, regardless of one existing or not.

## Offer Tribute
The human player can offer gold or spells to AI players to gain their favor. Spells can be offered if the other wizard is eligable for receiving them. Gold offered has to be proportional to the total amount of gold the AI wizard has, so bribing rich wizards is harder.
Four gold options are presented, which represent 25, 50, 75, and 100% of the amount of gold the wizard already has in the treasury, rounded down to the nearest 25, but a minimum of 25, 50, 75 or 100 gold even if the wizard has less than that amount.
If the player offers a spell, X* [random(1-8)+random(1-8)] will be added to the Visible Relations, and Peace Interest. 2+(random(1-8) is added to the Hidden Relation, but if it would exceed +30, it is set to +30. X equals 2 if the Visible Relation was below 0. Otherwise X = 10-(Visible Relation/10), so the better the relation is, the less the effect will be. However, the bonus is added to the human player's Peace Interest variable instead of the AI's, thus has no effect.
In the unofficial 1.50 patch, the amount added to Visible Relations and Peace Interest is X*[ random(1-8) +8] instead, making the option more reliable at producing meaningful results.
If the player offers gold, the Visible Relations improve by random(1-3)* Chosen Menu Item*(4*Chosen Menu Item)/10 where Chosen Menu Item is the gold option selected from 1 to 4. Offering gold has no effect on the Peace Interest and Hidden Relation variables.
In the unofficial 1.50 patch, the amount of Visible Relation gain is instead (1+random(3))*(Chosen Menu Item*2+1)/2 , for a gain of 3-6 points on the lowest, and 9-18 on the highest gold option.

## Exchange Spells
The human player can trade spells with AI wizards. For each spell received, exactly one spell has to be given. Both wizards need to be eligable for receiving the spells, and the spells have to be not yet researched for the receiving wizard, but already researched for the wizard offering it.
Although the Exchange Spells function calculates a complex formula for the relative value of spells in the trade acceptable based on relation variables, this value is not used in the game code. Instead, the spell needs to have an equal or lower Desirability Rating, in other words, the desired minimal value ratio is always 1. This might be a bug, or an intentional change to a poorly working feature, as a significant modifier could have resulted in extremely favorable trades towards the player. The unofficial patches do not change this behavior. It is important to mention that such a modifier does exist and functions when the AI offers a trade to the player in AI to Player Diplomacy, however, such an offer is only possible in the 1.50 unofficial patch, as bugs prevent it from happening otherwise. The patch also changes the modifier heavily as it, and the AI to Player spell trade function in general, was affected by many severe bugs, including the AI not receiving the spell, offers favoring the player more if the relation levels are worse, and only spells of exactly identical costs being possible to trade, which prevents even the previous bug from affecting the trade. See AI to Player Diplomacy for details.
Entering the Exchange Spells menu, if any spell trades are possible at all, Immediately reduces Peace Interest and Treaty Interest by 10, and Trade Interest by a random amount of 30-80. Leaving the menu, regardless of a success or failure of trading results in a loss of another 30 Treaty Interest, Trade Interest and Peace Interest. Additionally, if Trade Interest is still higher than +50, it is set to +50 instead. Prior to this, if the spell given to the AI has a higher research cost than the spell received, a strength 5 Diplomatic Reaction is generated, and a further 20 is subtracted from Treaty Interest, Trade Interest and Peace Interest. It is easy to see that trading spells normally causes a massive drop of success rates proposing treaties afterwards, and will usually end the audience itself in just 1-3 attempts. However, due to a bug in the Diplomatic Reaction process, if it is involved, the variables might reset to maximum values, allowing the audience and trading to continue. It is in theory possible to trade forever, at least until the valid spells run out, as long as trades favorable to the AI are done at least every third time. This bug is fixed in the 1.50 unofficial patch. Additionally the patch grants a +7 Diplomatic Reaction instead of +5 for favorable trades, these include trades of spells with an equal research cost instead of requiring strictly higher, and a major portion of temporal diplomatic relation losses are eliminated : When entering the menu, only 20-60 Trade Interest, 10 Peace Interest and 10 Treaty Interest is lost. When leaving, only 10 of each is lost. Finally, there is no additional loss when the trade was favorable to the AI player, and there is no requirement of Trade Interest being +50 or lower.
