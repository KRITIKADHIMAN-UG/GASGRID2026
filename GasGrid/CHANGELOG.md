# GasGrid update notes

## Updated academic prototype

- Added password recovery for customer and admin roles using expiring one-time codes. The code is displayed in the local demo UI; a real SMS/email provider is not configured.
- Passwords and recovery codes are stored as Werkzeug hashes. Existing legacy plaintext demo passwords are upgraded to hashes after a successful login.
- Added default demonstration orders and complaints, and expanded seed inventory for cylinders, staff, vehicles, facilities, safety kits, and regulators. Existing records are not overwritten.
- Added priority-based order totals: High +15%, Normal base price, Low -5%. The server recalculates the total and saves it.
- Clarified that each order has one delivery address; separate delivery destinations must be placed as separate orders.
- Replaced the simplified SRTF/Round Robin paths with actual scheduling simulations for FCFS, SJF, SRTF, Round Robin, and Priority. Round Robin preserves each quantum boundary in the Gantt chart.
- Time quantum is shown and sent only for Round Robin.
- Added scheduling unit tests and expanded code comments/documentation.

## Limitations

- This remains a local academic prototype. No real SMS/email delivery, payments, or delivery provider integration is configured.
- For production use, add rate limiting to recovery, verified messaging delivery, CSRF protection, a secure secret key, and a security review.
- Scheduling burst times are generated deterministically from order IDs to demonstrate algorithms; they are simulated values rather than measured delivery durations.


## Scheduling/data cleanup update
- Corrected scheduling inputs so multiple orders can be ready concurrently; burst estimates use order quantity and cylinder type rather than order ID alone.
- Added a labeled, horizontally scrollable Gantt timeline with per-segment duration and start/end ticks.
- Expanded fresh-install demo data to 120 customers and 360 refill orders, plus sample complaints.
- Reduced inventory to four core resource rows and remove legacy extras at startup.
- Added tests for algorithm differentiation, SRTF preemption, and arrival-time handling.


## Controlled dataset and admin filtering
- Replaced the earlier 120-customer/360-order seed with a one-time migration to exactly 50 customers, 50 orders, 50 inventory records, and 50 complaints. Old prototype demo rows are purged once; subsequent app restarts preserve activity.
- Added customer account status (Active/Inactive/Suspended), admin status updates, status filtering, and login blocking for non-active customer accounts.
- Added admin search and status filters for customers and orders.
- Simplified the package layout so there is one unambiguous project root, reducing the chance of launching stale nested files.
