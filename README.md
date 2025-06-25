Blythe Mountain Light
===========

## What problem needed solving?

Part of finding a joy in electronics was talking about it. A lot. My coworker has a similar, albeit inferior, hobby of his own in flashlights. After talking his ear off enough about my craving for new projects he threw out a great new challenge for both of us. He told me his father and grand father had always wanted to put a light at the top a hill in Blythe, CA that could be seen anywhere in town. The solution would need to be rugged and self-sustainable as getting out to the installation site would be a hike and there would be no local power.

For those unfamiliar with Blythe, it is located on the California/Arizona boarder, it’s main exports are dirt, blue grass, and skin cancer, and it’s town slogan is “Take this exit, that last sign said it has a Carls Jr.”

![image](https://github.com/user-attachments/assets/e3512aa1-0e21-4e60-959f-5dbd3d90cfb6)
<em>Not pictured: skin cancer.</em>

Why Blythe? My coworker was raised in Blythe and installing this light would put a mark on his home town. He painted me scenarios in which parents would tell their kids to come home when “The Light” was on, or a local would comment on “The Light” turning on earlier as winter approached. While those scenarios sounded nice, I couldn’t help but wonder if we were really building a guiding star for any escaping family members currently serving time in Blythe’s Chuckawalla Valley State Prison (ala the Underground Railroad).


![image](https://github.com/user-attachments/assets/0465766e-3220-4e8d-a2f6-ba29ad9ddefd)

## Why This Project?
This project was interesting from the start, it had power management, sensors, and a bit of a punk rock purpose. I had been to Blythe and for all the grief I give it, it was exciting to have a project that could be visible to the whole town.

## What I wanted to Learn
This project was pretty much all power management: Controlling the light’s state, brokering the interaction and charging between the solar panels and the batteries, ensuring the batteries aren’t over discharged, and minimizing the microcontroller’s current draw as much as possible.

## Development Planning
My coworker brought a lot of knowledge to the table from his flashlight experiences including battery management, efficient LEDs, and protection circuits. His flashlight hobby also provided insight into the seemingly boundless range a man can have for mundane hobbies, but given the context of this project, this is neither here nor there.  After discussing the project requirements we divvied up the project duties as followed:

## Project Struggles and Lessons Learned
**Problem:** The first struggle I ran into was picking a trigger for switching the status the light.

**Solution:** The idea to put the device on a schedule was thrown out of the running almost instantly. My coworker’s use case had the light reacting to nightfall. On top of this, to ensure the device correctly stayed in sync an external timer would need to be introduced, which would then need it’s own power source; that too would need regular replacing. Though if I had purchased an external timer, it likely would have been [this one](https://www.sparkfun.com/products/10160).

After that quick decision, I started looking at UV sensors. UV made sense to me after the Bathroom Sign because of the issues with ambient lighting near the sensor. My thought at the time was that the local of the LED wouldn’t need to be taken into account relative to the sensor because of the LED’s low UV output. I bought a [UV sensor from SparkFun](https://www.sparkfun.com/products/12705) and started experimenting.

Unfortunately it became clear that UV light is not really one-to-one with daylight so much as it is with direct sun exposure. Given that my typical day includes sitting at a desk inside, it took me more times than I care to admit to realize my sensor wasn’t broken when testing it after work, I just was testing it after the sun quit doling out UV for the day.  It did however shed light (puns!) on why I can still be so pale after living in Southern California for nearly three years.

I finally relented and made the move toward the same type of photoresistor I used in the Bathroom Sign. This sensor ended up being perfect after adding a 470k Ω pull-down resistor.

**Problem:** Choosing a microcontroller

**Solution:** Knowing that I had to eventually make progress towards improving the power consumption of the microcontroller, I knew going small was the first consideration to make. I also knew having strict voltage requirements would complicate My coworker’s role in battery configurations. Lastly the device would likely grow in scope as the project progressed, so the microcontroller’s responsibilities would need to be able to expand as needed. All of these factors considered, I ended up with the Arduino Pro Mini (5v/16Mhz

![image](https://github.com/user-attachments/assets/28c24c40-a09a-4f79-aca7-951ff683f34e)

**Problem:** “The sequence looks good, we’re just over budget on the amperage.”

**Solution:** Truth be told, this microcontroller’s power consumption would have been fine out of the box, but I wasn’t about to let that stop me from having fun! I began pouring over every article and forum post I could find about power saving options with the arduino, and in the end it came down to four methods: Built-in sleep modes, underclocking, physical alterations, and clever coding.

### Built-In Sleep Modes
Learning about the built in sleep modes was a trip into reading datasheets, understanding timers, and moving around individual bits to change the microprocessor’s settings. Ultimately this deserves it’s own post, so for now I’ll just say that learning these elements were the most fun I had learning in a long while.

### Underclocking
It should not have surprised me to learn it’s possible and relatively easy to change the arduino’s clock speed. This is another concept that deserves it’s own post, I will make sure to do a through write up on this process.

### Physical Alterations
One of the articles I found suggested I remove the power LED that comes mounted on the arduino. After reading that this LED draws roughly 3mA I quickly broke out the x-acto and went to work on it’s leads.

### Clever Coding
Without doing damage to my spine, I feel pretty good with how my code made use of it’s time in relation to it’s current draw. I made use of the watchdog timer, powered down the arduino, underclocked the processor, and only briefly drew power to check for any changes in daylight. As I mentioned before, this process will get it’s own post, for the time being my code can be found at the bottom of this post.

**Problem:** Choosing a relay.

**Solution:** Finding the right relay for this project ended up being more time consuming than I first thought. I hadn’t dealt with a relay in my projects before this and finding one that was the right size seemed to be where to start. My project box was a 3x2x1″ project box from [Radio Shack](https://www.radioshack.com/products/radioshack-project-enclosure-3x2x1?variant=5717249605) and it was my goal to not exceed this box.

After further research into relays it was clear I also needed to take into account the output voltage my controller was capable of supplying. This constraint, and my desire for expandability, lead me to search for 2 Channel 5v Relay Modules. I ordered one from Amazon and started to make a proof of concept.

Unfortunately as I began combining all the concepts I had been collecting, one flaw came to light: The relay I had chosen would need constant power to turn on the light. This was unacceptable given my work to bring current draw down into microamps. The solution was simple, finding a small 2 channel latching relay. I found a good one [here](http://www.seeedstudio.com/wiki/Grove_-_2-Coil_Latching_Relay).

To use the latching relay you must send rising or falling voltages to the relay’s data line. To do this, you’d either digitalwrite to your pin from LOW to HIGH or High to Low. This was easy enough, but then I thought, “If I send a rising voltage operation to turn on the light (LOW to HIGH) how go back to no voltage output without the relay thinking I was sending a falling (HIGH to LOW) voltage?” Well, the wiki for the relay doesn’t say specifically, but I determined the solution was to simply stop giving power to the whole relay and then write LOW to the data pin.

**Problem:** I wanted to only supply power to my components briefly and deliberately.

**Solution:** I’m not sure if this is a common arduino practice, but I simply used another pin as power for my photoresistor and relay. I read the maximum current draw from the Pro Mini’s pins was 40mA, so I would toggle voltage from that pin on and off to give power to the components just long enough to get a reading/change state and then I discontinued the power.

![image](https://github.com/user-attachments/assets/e298e7c5-f5ec-440e-ad66-2d11e46de577)

![image](https://github.com/user-attachments/assets/ad26d939-b71a-4d43-b46e-54201cb07c11)

## Future Improvements
Already now that Phase 1 is complete, we have started started talking about increasing the box’s scope to include battery discharge readings. I also have ideas for how to handle dust and stand buildup if that becomes a problem. From a code stand point, I have considered creating a power saving library to bring easier power saving to my other projects.
