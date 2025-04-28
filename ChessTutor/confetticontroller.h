/*
 * confetticontroller.h
 *
 * Defines the ConfettiController class, which manages confetti
 * particle simulation and rendering for both capture and puzzle
 * completion celebrations on the chess board.
 * Uses Box2D for realistic physics and QPainter for drawing.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#ifndef CONFETTICONTROLLER_H
#define CONFETTICONTROLLER_H

#include "qcolor.h"
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <vector>
#include <Box2D/Box2D.h>

class ChessBoard;
class QPainter;

/**
 * ConfettiController
 *
 * Inherits QObject to provide timed physics updates and drawing hooks.
 * - Simulates confetti particles using Box2D physics (gravity, bounces, damping).
 * - Renders particles as colored squares via QPainter on the ChessBoard.
 * - Supports random bursts or targeted spawns at specific board coordinates.
 */
class ConfettiController : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param view   Pointer to the ChessBoard widget for redraw triggers.
     * @param parent Optional parent QObject for memory management.
     */
    explicit ConfettiController(ChessBoard *view, QObject *parent = nullptr);

    /**
     * Destructor
     *
     * Cleans up all Box2D bodies and associated resources.
     */
    ~ConfettiController() override;

    /**
     * spawn
     *
     * Create a burst of confetti particles from the top of the board.
     *
     * @param count Number of particles to spawn.
     */
    void spawn(int count);

    /**
     * spawnAt
     *
     * Create a burst of confetti particles at a specific world coordinate
     * and apply random impulses.
     *
     * @param worldPos 2D position in Box2D world units (meters).
     * @param count    Number of particles to spawn (default: 100).
     */
    void spawnAt(const b2Vec2& worldPos, int count = 100);

    /**
     * draw
     *
     * Render all active confetti particles as fading, rotating squares.
     *
     * @param painter QPainter reference used for drawing onto ChessBoard.
     */
    void draw(QPainter& painter) const;

    /// Pixel scaling factor: Box2D meters to screen pixels
    static constexpr float kScale = 50.0f;

signals:
    /**
     * spawned
     *
     * Emitted after a spawn call to notify listeners of new particles.
     *
     * @param count Number of particles created.
     */
    void spawned(int count);

private:
    /**
     * stepPhysics
     *
     * Advance the Box2D simulation, handle wall bounces, and cull expired particles.
     */
    void stepPhysics();

    struct Particles {
        b2Body*  body;       ///< Box2D body representing the particle
        QColor   color;      ///< Render color
        float    size;       ///< Half-size (meters)
        float    birthTime;  ///< Timestamp of creation (seconds)
    };

    b2World                world;   ///< Physics world with gravity
    QTimer                 timer;   ///< Periodic update timer (60Hz)
    QElapsedTimer          m_clock; ///< Exact elapsed time for lifetimes
    ChessBoard*            board;   ///< Target chess board for redraws
    std::vector<Particles> m_parts; ///< Active confetti particles

    // Lifespan parameters (seconds)
    static constexpr float kLifeSpan     = 3.0f;
    static constexpr float kFadeDuration = 1.0f;

    /// Color palette for particles
    static const std::vector<QColor> kPalette;

public slots:
    /**
     * onSpawn
     *
     * Slot wrapper to call spawn() via Qt signal.
     *
     * @param count Number of particles to create.
     */
    void onSpawn(int count) { spawn(count); }

    /**
     * onSpawnAt
     *
     * Slot wrapper to call spawnAt() via Qt signal.
     *
     * @param x     X coordinate in physics world.
     * @param y     Y coordinate in physics world.
     * @param count Number of particles to create.
     */
    void onSpawnAt(float x, float y, int count) { spawnAt(b2Vec2(x, y), count); }
};

#endif // CONFETTICONTROLLER_H
