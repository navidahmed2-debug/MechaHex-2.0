"""
===================================================================
🏆 NAVID'S PERSONAL SKILLS DEVELOPMENT SANDBOX
Project: MechaHex 2.0
Focus: Mastering Spatial Transformations, Array Traversal & Vector Math
===================================================================
Use this isolated script to experiment with advanced Python logic,
matrix mechanics, or data filtering before implementing them in production.
"""

def practice_coordinate_transformation():
    print("[SANDBOX] Learning Spatial Vector Math...")
    # Example: Simulating a 3D target coordinate vector from the Vision Team
    # Format: [X_distance, Y_lateral, Z_elevation]
    mock_vision_target = [12.5, -4.2, 8.7]
    
    print(f"Raw Input Target Vector: {mock_vision_target}")
    
    # Practice parsing coordinates for custom inverse kinematics calculations
    x, y, z = mock_vision_target
    print(f"Parsed coordinates -> Spatial X: {x}, Spatial Y: {y}, Elevation Z: {z}")
    
    # TODO: Add your custom matrix multiplier logic here as you learn it this week!

def practice_telemetry_stream():
    print("\n[SANDBOX] Learning Hexapod Gait Delay & Frequency Logic...")
    # Practice handling time arrays for smooth leg movement intervals
    gait_intervals = [0.1, 0.2, 0.3, 0.4]
    
    # Doubling speed scaling factor practice
    optimized_intervals = [t / 2 for t in gait_intervals]
    print(f"Original Gait Cycle: {gait_intervals}")
    print(f"Optimized Phase-Shifted Cycle: {optimized_intervals}")

if __name__ == "__main__":
    print("=== STARTING PERSONAL SKILL TRAINER ===")
    practice_coordinate_transformation()
    practice_telemetry_stream()
    print("=======================================")
