import random
import time

import numpy as np
from sklearn.cluster import MiniBatchKMeans

# =========================
# Utility Functions
# =========================


def cosine_distance(a, b):
    a = a / np.linalg.norm(a)
    b = b / np.linalg.norm(b)
    return 1 - np.dot(a, b)


def fake_embedding(text, dim=128):
    """Deterministic pseudo-embedding (stand-in for real encoder)."""
    random.seed(hash(text) % (2**32))
    return np.array([random.random() for _ in range(dim)])


# =========================
# Learned Primitive Engine
# =========================


class LearnedPrimitiveEngine:
    def __init__(self, n_primitives=6, dim=128):
        self.n_primitives = n_primitives
        self.dim = dim
        self.kmeans = MiniBatchKMeans(
            n_clusters=n_primitives,
            batch_size=32,
            random_state=42,
        )
        self.initialized = False
        self.last_centroids = None

    def partial_fit(self, embeddings):
        self.kmeans.partial_fit(embeddings)
        self.initialized = True

    def drift(self):
        if self.last_centroids is None:
            self.last_centroids = self.kmeans.cluster_centers_.copy()
            return 0.0

        drift_vals = [
            cosine_distance(a, b)
            for a, b in zip(self.last_centroids, self.kmeans.cluster_centers_)
        ]
        self.last_centroids = self.kmeans.cluster_centers_.copy()
        return float(np.mean(drift_vals))

    def reinitialize(self, seed_embeddings=None):
        self.kmeans = MiniBatchKMeans(
            n_clusters=self.n_primitives,
            batch_size=32,
            random_state=42,
        )
        self.initialized = False
        self.last_centroids = None
        if seed_embeddings is not None:
            self.partial_fit(seed_embeddings)
        print("⚠️  Primitive engine re-initialized")


# =========================
# Lyapunov Controller
# =========================


def lyapunov_controller(confidence, depth, drift, max_depth=8, max_drift=0.3):
    V = (
        (1 - confidence) ** 2
        + (depth / max_depth) ** 2
        + (drift / max_drift) ** 2
    )

    dV = -0.6 * confidence + 0.25 * drift + 0.15 * (depth / max_depth)
    safe = dV <= 0
    return safe, V


# =========================
# TR-GI Agent
# =========================


class TRGIAgent:
    def __init__(self, agent_id):
        self.id = agent_id
        self.lpe = LearnedPrimitiveEngine()
        self.depth = 0
        self.last_conf = 0.0
        self.last_drift = 0.0

    def step(self, text):
        emb = fake_embedding(text)
        self.lpe.partial_fit([emb])

        self.last_drift = self.lpe.drift()
        self.last_conf = max(0.0, 1.0 - self.last_drift - 0.05 * self.depth)
        self.depth += 1

        safe, V = lyapunov_controller(
            self.last_conf,
            self.depth,
            self.last_drift,
        )

        return {
            "agent": self.id,
            "confidence": self.last_conf,
            "drift": self.last_drift,
            "depth": self.depth,
            "V": V,
            "safe": safe,
        }


# =========================
# Multi-Agent Arbiter
# =========================


class TRGIArbiter:
    def __init__(self, agents):
        self.agents = agents

    def evaluate(self):
        confs = [a.last_conf for a in self.agents]
        drifts = [a.last_drift for a in self.agents]

        avg_conf = float(np.mean(confs))
        max_drift = float(np.max(drifts))

        if avg_conf > 0.8 and max_drift < 0.2:
            decision = "continue"
        elif max_drift > 0.3:
            decision = "reinit"
        else:
            decision = "halt"

        return decision, avg_conf, max_drift


# =========================
# Main Loop (Demo)
# =========================


def run_demo():
    agents = [TRGIAgent(i) for i in range(3)]
    arbiter = TRGIArbiter(agents)

    prompt = "Explain recursion stability in intelligent systems."

    print("\n🧠 TR-GI SYSTEM START\n")

    for step in range(1, 15):
        print(f"\n--- STEP {step} ---")

        for agent in agents:
            result = agent.step(prompt)
            print(
                f"Agent {agent.id} | "
                f"Conf={result['confidence']:.3f} | "
                f"Drift={result['drift']:.3f} | "
                f"V={result['V']:.3f}"
            )

        decision, avg_conf, max_drift = arbiter.evaluate()

        print(
            f"\n[ARBITER] avg_conf={avg_conf:.3f} "
            f"max_drift={max_drift:.3f} "
            f"→ decision={decision.upper()}"
        )

        if decision == "reinit":
            for agent in agents:
                agent.lpe.reinitialize()
                agent.depth = 0

        if decision == "halt":
            print("\n🛑 System halted for stability")
            break

        time.sleep(0.5)

    print("\n✅ TR-GI demo completed\n")


if __name__ == "__main__":
    run_demo()
