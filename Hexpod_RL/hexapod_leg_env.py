import pybullet as p
import pybullet_data
import gym
import numpy as np
from gym import spaces

class HexapodLegEnv(gym.Env):
    def __init__(self):
        super(HexapodLegEnv, self).__init__()
        self.physics_client = p.connect(p.GUI)  # Use DIRECT mode for faster training
        p.setAdditionalSearchPath(pybullet_data.getDataPath())  # PyBullet data path

        urdf_path = r"C:\Users\zuhay\Documents\Arduino\Hexapod\Hexpod_RL\hexapod_leg.urdf"
        self.leg_id = p.loadURDF(urdf_path)

        # Define action and observation spaces
        self.action_space = spaces.Box(low=-1, high=1, shape=(2,), dtype=np.float32)
        self.observation_space = spaces.Box(low=-3.14, high=3.14, shape=(2,), dtype=np.float32)

    def reset(self):
        p.resetSimulation()
        urdf_path = r"C:\Users\zuhay\Documents\Arduino\Hexapod\Hexpod_RL\hexapod_leg.urdf"
        self.leg_id = p.loadURDF(urdf_path)
        return self._get_observation()

    def step(self, action):
        # Scale actions to fit motor torque limits
        max_torque = 10.0
        scaled_action = np.clip(action, -1, 1) * max_torque
        p.setJointMotorControlArray(self.leg_id, [0, 1], p.TORQUE_CONTROL, forces=scaled_action)

        # Simulate physics
        p.stepSimulation()

        # Get observations
        obs = self._get_observation()

        # Calculate reward
        reward = -np.sum(np.square(obs))
        done = False  # End condition (optional)
        return obs, reward, done, {}

    def _get_observation(self):
        joint_states = p.getJointStates(self.leg_id, [0, 1])
        return np.array([state[0] for state in joint_states])

    def close(self):
        p.disconnect(self.physics_client)

# Test the environment
if __name__ == "__main__":
    env = HexapodLegEnv()
    obs = env.reset()
    print(f"Initial Observation: {obs}")

    for _ in range(10):
        action = env.action_space.sample()
        obs, reward, done, info = env.step(action)
        print(f"Action: {action}, Observation: {obs}, Reward: {reward}")

    env.close()
