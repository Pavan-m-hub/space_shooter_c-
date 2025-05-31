# Space Shooter

![Space Shooter Game](assets/images/background.jpg)

A fast-paced, action-packed space shooter game built with C++ and SFML. Battle through waves of enemies, collect power-ups, upgrade your weapons, and face challenging boss battles across multiple levels.

## Features

### Dynamic Gameplay
- **Multiple Enemy Types**: Face different enemies with unique behaviors and attributes
- **Boss Battles**: Epic confrontations with powerful boss ships
- **Power-Up System**: Collect various power-ups to enhance your ship
- **Weapon Upgrades**: Progress from basic weapons to powerful laser beams
- **Level Progression**: Battle through 5 increasingly difficult levels

### Visual Effects
- **Particle System**: Dynamic explosion effects
- **Shield Visuals**: See your shield status with visual feedback
- **Animated Sprites**: Smooth animations for game elements
- **Warning Effects**: Visual alerts for boss encounters

### Game Elements
- **Health System**: Monitor and maintain your ship's health
- **Shield System**: Temporary protection from enemy attacks
- **Score System**: Earn points by destroying enemies and collecting power-ups
- **Sound Effects**: Immersive audio for all game actions

## Controls

- **Arrow Keys**: Move your ship (Up, Down, Left, Right)
- **Space**: Fire weapons
- **Enter**: Start game (from main menu)
- **R**: Restart game (after game over or victory)

## Power-Ups

| Type | Effect |
|------|--------|
| Health | Restores 25% of your ship's health |
| Shield | Activates a protective shield that absorbs damage |
| Weapon Upgrade | Improves your weapon to the next level |
| Score Boost | Instantly adds 50 points to your score |

## Weapon Types

| Level | Type | Description |
|-------|------|-------------|
| 1 | Basic | Single bullet with moderate damage |
| 2 | Double | Two bullets fired simultaneously |
| 3 | Triple | Three bullets in a spread pattern |
| 4 | Laser | Continuous beam that damages all enemies it touches |

## Enemy Types

| Type | Characteristics |
|------|----------------|
| Basic | Balanced health and speed |
| Fast | Quick movement but lower health |
| Tanky | Slow movement but higher health |
| Boss | Unique movement patterns, high health, and special attacks |

## Installation

### Prerequisites
- C++17 compatible compiler
- SFML 2.6.2 or newer

### Building from Source

1. Clone the repository:
```
git clone https://github.com/yourusername/space-shooter.git
cd space-shooter
```

2. Compile the game:
```
g++ -std=c++17 main.cpp -o output/main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

3. Run the game:
```
./output/main
```

### macOS Specific Instructions

If you're using Homebrew:
```
brew install sfml
g++ -std=c++17 main.cpp -o output/main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

## Game Structure

The game is built using object-oriented programming principles with the following key classes:

- **Game**: Main game controller managing states and resources
- **Player**: Player ship with health, weapons, and movement
- **Enemy**: Base class for all enemy types
- **Bullet/Laser**: Projectile weapons
- **PowerUp**: Various collectible items
- **Particle/Explosion**: Visual effects
- **Level**: Manages game progression and difficulty

## Development

The game was developed using:
- C++17
- SFML (Simple and Fast Multimedia Library)
- Object-oriented design principles

## Future Enhancements

Potential future improvements:
- Multiplayer mode
- Additional weapon types
- More enemy varieties
- Persistent high scores
- Customizable ships
- Additional levels and environments

## Credits

- Game assets from [OpenGameArt.org](https://opengameart.org/)
- Built with [SFML](https://www.sfml-dev.org/)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

Enjoy the game! If you have any questions or suggestions, please open an issue on the repository.
