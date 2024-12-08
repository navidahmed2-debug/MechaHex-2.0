from stable_baselines3 import PPO
from hexapod_leg_env import HexapodLegEnv

env = HexapodLegEnv()
model = PPO("MlpPolicy", env, verbose=1)
model.learn(total_timesteps=10000)

# Save the model
model.save("ppo_hexapod_leg")

# Load and test
model = PPO.load("ppo_hexapod_leg")
obs = env.reset()
for _ in range(20):
    action, _states = model.predict(obs)
    obs, reward, done, info = env.step(action)
    print(f"Action: {action}, Observation: {obs}, Reward: {reward}")
