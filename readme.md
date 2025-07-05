Sprint 4:
What's New in the Game:
→ Added "coin.wav" sound when collecting coins
→ Added "shock.wav" sound when hitting obstacles
→ Implemented stack system for coin collection/undo
→ Press 'U' to undo last coin collection (returns coin, deducts 10 points)
→ Enhanced player feedback with sounds and visuals

How the New Features Work:
→ Sound System:

Initialized in main() using S2D_CreateSound()

Triggered during collisions using S2D_PlaySound()

→ Stack Implementation:

Stack struct tracks collected coins

push() adds coins when collected

pop() removes coins when undoing

Technical Improvements:

Proper sound memory management

Bounds-checked stack operations

Seamless integration with existing code

Enhanced player experience with audio-visual feedback

All Sprint 3 functionality remains unchanged and fully operational.