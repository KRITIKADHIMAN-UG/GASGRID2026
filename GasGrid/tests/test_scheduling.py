"""Unit tests for the five CPU scheduling policies used by GasGrid."""
import unittest
from app import _schedule_jobs


class SchedulingTests(unittest.TestCase):
    def setUp(self):
        self.jobs = [
            {"pid": "P1", "order_id": 1, "arrival": 0, "burst": 5, "priority": 2},
            {"pid": "P2", "order_id": 2, "arrival": 1, "burst": 2, "priority": 1},
            {"pid": "P3", "order_id": 3, "arrival": 2, "burst": 1, "priority": 3},
        ]

    def test_all_algorithms_finish_each_job_and_conserve_cpu_time(self):
        for algorithm in ("FCFS", "SJF", "SRTF", "RR", "PRIORITY"):
            with self.subTest(algorithm=algorithm):
                results, gantt = _schedule_jobs(self.jobs, algorithm, 2)
                self.assertEqual(len(results), len(self.jobs))
                self.assertEqual(sum(x["end"] - x["start"] for x in gantt), sum(j["burst"] for j in self.jobs))
                for result in results:
                    self.assertEqual(result["turnaround"], result["completion"] - result["arrival"])
                    self.assertEqual(result["waiting"], result["turnaround"] - result["burst"])

    def test_round_robin_respects_quantum_boundaries(self):
        _, gantt = _schedule_jobs(self.jobs, "RR", 2)
        self.assertTrue(all(0 < segment["end"] - segment["start"] <= 2 for segment in gantt))

    def test_srtf_preempts_a_longer_running_job(self):
        _, gantt = _schedule_jobs(self.jobs, "SRTF", 2)
        self.assertEqual(gantt[0], {"pid": "P1", "start": 0, "end": 1})
        self.assertEqual(gantt[1]["pid"], "P2")

    def test_fcfs_sjf_and_srtf_have_algorithm_specific_execution_traces(self):
        _, fcfs = _schedule_jobs(self.jobs, "FCFS")
        _, sjf = _schedule_jobs(self.jobs, "SJF")
        _, srtf = _schedule_jobs(self.jobs, "SRTF")
        fcfs_order = [segment["pid"] for segment in fcfs]
        sjf_order = [segment["pid"] for segment in sjf]
        srtf_order = [segment["pid"] for segment in srtf]
        self.assertNotEqual(fcfs_order, sjf_order)
        self.assertNotEqual(fcfs_order, srtf_order)
        self.assertEqual(fcfs[0], {"pid": "P1", "start": 0, "end": 5})
        self.assertEqual(srtf[0], {"pid": "P1", "start": 0, "end": 1})
        self.assertEqual(srtf[1]["pid"], "P2")

    def test_arrival_times_are_respected(self):
        jobs = [
            {"pid": "late", "order_id": 2, "arrival": 4, "burst": 2, "priority": 2},
            {"pid": "early", "order_id": 1, "arrival": 0, "burst": 1, "priority": 2},
        ]
        results, gantt = _schedule_jobs(jobs, "FCFS")
        self.assertEqual(gantt[0], {"pid": "early", "start": 0, "end": 1})
        self.assertEqual(gantt[1], {"pid": "late", "start": 4, "end": 6})
        late = next(result for result in results if result["pid"] == "late")
        self.assertEqual(late["waiting"], 0)


if __name__ == "__main__":
    unittest.main()
