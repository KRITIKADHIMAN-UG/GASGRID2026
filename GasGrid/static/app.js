const $ = id => document.getElementById(id);
let toastTimer;
let adminOrdersCache=[], adminCustomersCache=[];
function toast(msg){const el=$("toast");if(!el)return;el.textContent=msg;el.classList.add("show");clearTimeout(toastTimer);toastTimer=setTimeout(()=>el.classList.remove("show"),2800);}
function esc(v){return String(v??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));}
async function api(url,options={}){const r=await fetch(url,{...options,headers:{"Content-Type":"application/json",...(options.headers||{})}});let d={};try{d=await r.json();}catch{}if(!r.ok)throw new Error(d.error||"Request failed.");return d;}
function feedback(id,msg,kind=""){const el=$(id);if(el){el.textContent=msg;el.className="feedback "+kind;}}
async function logout(){await api("/api/logout",{method:"POST",body:"{}"});location.href="/login";}
if($("logout-btn"))$("logout-btn").addEventListener("click",logout);
function renderStats(stats){const items=[["Orders",stats.orders],["Pending",stats.pending],["Delivered",stats.delivered],...(window.GASGRID_ROLE==="admin"?[["Customers",stats.customers],["Open complaints",stats.complaints]]:[["My complaints",stats.complaints]])];$("stats").innerHTML=items.map(([k,v])=>`<div class="stat-card"><span>${esc(k)}</span><strong>${v??0}</strong></div>`).join("");}
async function loadDashboard(){
 try{const d=await api("/api/dashboard");renderStats(d.stats);
 if(window.GASGRID_ROLE==="user"){
  $("orders-body").innerHTML=d.orders.length?d.orders.map(o=>`<tr><td>#${o.order_id} / P${o.order_id}</td><td>${esc(o.created_at)}</td><td>${esc(o.cylinder_type)}</td><td>${o.quantity}</td><td>₹${o.total_amount}</td><td>${esc(o.delivery_address)}</td><td>${["","High","Normal","Low"][o.priority]}</td><td><span class="status">${esc(o.status)}</span></td></tr>`).join(""):`<tr><td colspan="8">No refill orders yet.</td></tr>`;
  $("complaints-body").innerHTML=d.complaints.length?d.complaints.map(c=>`<tr><td>#${c.complaint_id}</td><td>${esc(c.category)}</td><td>${esc(c.details)}</td><td>${esc(c.status)}</td><td>${esc(c.created_at)}</td></tr>`).join(""):`<tr><td colspan="5">No complaints yet.</td></tr>`;
  const history=await api("/api/schedule/history");$("user-schedule-history").innerHTML=history.length?history.map(x=>`<div class="history-item"><b>${esc(x.pid)}</b> · ${esc(x.algorithm)} · waiting ${x.waiting_time} · turnaround ${x.turnaround_time}</div>`).join(""):"Your order's scheduling results will appear here after the admin runs the scheduler.";
 }else{
  adminOrdersCache=d.orders; adminCustomersCache=d.customers;
  renderAdminOrders(); renderAdminCustomers();
  $("inventory-body").innerHTML=d.inventory.map(i=>`<tr><td>${esc(i.item_name)}</td><td>${esc(i.category)}</td><td>${i.available} ${esc(i.unit)}</td><td><input id="stock-${i.item_id}" type="number" min="-500" max="500" value="1" class="stock-input"><button class="small-btn" onclick="adjustStock(${i.item_id})">Apply +/-</button></td></tr>`).join("");

  $("complaints-body").innerHTML=d.complaints.map(c=>`<tr><td>#${c.complaint_id}</td><td>${esc(c.customer)}</td><td>${esc(c.phone)}</td><td>${esc(c.category)}</td><td>${esc(c.details)}</td><td>${esc(c.status)}</td><td><select id="complaint-${c.complaint_id}">${["Open","In Progress","Resolved","Rejected"].map(s=>`<option ${s===c.status?"selected":""}>${s}</option>`).join("")}</select><button class="small-btn" onclick="updateComplaint(${c.complaint_id})">Save</button></td></tr>`).join("")||`<tr><td colspan="7">No complaints yet.</td></tr>`;
 }
 }catch(e){toast(e.message);if(e.message.includes("log in"))location.href="/login";}
}
function renderAdminOrders(){
 const q=($("order-search")?.value||"").trim().toLowerCase(), status=$("order-status-filter")?.value||"";
 const rows=adminOrdersCache.filter(o=>(!status||o.status===status)&&(!q||[o.order_id,o.customer,o.phone,o.delivery_address,o.cylinder_type].some(v=>String(v??"").toLowerCase().includes(q))));
 $("orders-body").innerHTML=rows.length?rows.map(o=>`<tr data-status="${esc(o.status)}"><td>#${o.order_id}</td><td>${esc(o.customer)}</td><td>${esc(o.phone)}</td><td>${esc(o.cylinder_type)}</td><td>${o.quantity}</td><td>₹${o.total_amount}</td><td>${esc(o.delivery_address)}</td><td>${esc(o.status)}</td><td><select id="status-${o.order_id}">${["Pending","Scheduled","Out for Delivery","Delivered","Cancelled"].map(s=>`<option ${s===o.status?"selected":""}>${s}</option>`).join("")}</select><button class="small-btn" onclick="updateOrder(${o.order_id})">Save</button></td></tr>`).join(""):`<tr><td colspan="9">No matching orders.</td></tr>`;
}
function renderAdminCustomers(){
 const q=($("customer-search")?.value||"").trim().toLowerCase(), status=$("customer-status-filter")?.value||"";
 const rows=adminCustomersCache.filter(c=>(!status||(c.account_status||"Active")===status)&&(!q||[c.user_id,c.name,c.phone,c.address].some(v=>String(v??"").toLowerCase().includes(q))));
 $("customers-body").innerHTML=rows.length?rows.map(c=>`<tr><td>${c.user_id}</td><td>${esc(c.name)}</td><td>${esc(c.phone)}</td><td>${esc(c.address)}</td><td><select id="customer-status-${c.user_id}">${["Active","Inactive","Suspended"].map(s=>`<option ${s===(c.account_status||"Active")?"selected":""}>${s}</option>`).join("")}</select><button class="small-btn" onclick="updateCustomerStatus(${c.user_id})">Save</button></td><td>${esc(c.created_at)}</td></tr>`).join(""):`<tr><td colspan="6">No matching customers.</td></tr>`;
}
async function updateCustomerStatus(id){try{const status=$("customer-status-"+id).value;const d=await api("/api/admin/customer-status",{method:"POST",body:JSON.stringify({user_id:id,status})});toast(d.message);await loadDashboard();}catch(e){toast(e.message);}}
for(const id of ["customer-search","customer-status-filter"])if($(id))$(id).addEventListener(id.endsWith("search")?"input":"change",renderAdminCustomers);
for(const id of ["order-search","order-status-filter"])if($(id))$(id).addEventListener(id.endsWith("search")?"input":"change",renderAdminOrders);
async function updateOrder(id){try{const status=$("status-"+id).value;const d=await api("/api/admin/order-status",{method:"POST",body:JSON.stringify({order_id:id,status})});toast(d.message);await loadDashboard();}catch(e){toast(e.message);}}
async function adjustStock(id){try{const change=Number($("stock-"+id).value);if(!Number.isInteger(change)||change===0)throw Error("Enter a non-zero whole-number stock adjustment.");const d=await api("/api/admin/inventory",{method:"POST",body:JSON.stringify({item_id:id,change})});toast(d.message);await loadDashboard();}catch(e){toast(e.message);}}
async function updateComplaint(id){try{const status=$("complaint-"+id).value;const d=await api("/api/admin/complaint-status",{method:"POST",body:JSON.stringify({complaint_id:id,status})});toast(d.message);await loadDashboard();}catch(e){toast(e.message);}}
const orderForm=$("order-form");
if(orderForm){
 const type=orderForm.elements.cylinder_type, qty=orderForm.elements.quantity, priority=orderForm.elements.priority;
 // Keep the client-side estimate aligned with the server's authoritative pricing rule.
 function updatePrice(){
  const unitPrice=type.value==="Domestic"?950:2200;
  const multiplier={"1":1.15,"2":1.00,"3":0.95}[priority.value]||1;
  const base=unitPrice*Number(qty.value), total=Math.round(base*multiplier);
  $("estimated-total").textContent="₹"+total;
  const labels={"1":"High priority · +15%","2":"Normal priority · base price","3":"Low priority · −5%"};
  const note=$("price-note");if(note)note.textContent=`${labels[priority.value]}. One order has one delivery address; create separate orders for separate locations. Demo prices only; payment is not processed.`;
 }
 type.addEventListener("change",updatePrice);qty.addEventListener("change",updatePrice);priority.addEventListener("change",updatePrice);
 orderForm.addEventListener("submit",async e=>{
  e.preventDefault();const savedForm=e.currentTarget;const data=Object.fromEntries(new FormData(savedForm).entries());data.quantity=Number(data.quantity);data.priority=Number(data.priority);
  feedback("order-feedback","Saving refill order...");
  try{const d=await api("/api/orders",{method:"POST",body:JSON.stringify(data)});feedback("order-feedback",`${d.message} Order #${d.order_id}; amount ₹${d.total_amount}.`,"success");savedForm.reset();updatePrice();toast(`Order #${d.order_id} saved`);await loadDashboard();}
  catch(err){feedback("order-feedback",err.message,"error");}
 });
}
const complaintForm=$("complaint-form");
if(complaintForm)complaintForm.addEventListener("submit",async e=>{
 e.preventDefault();const savedForm=e.currentTarget;const data=Object.fromEntries(new FormData(savedForm).entries());feedback("complaint-feedback","Submitting complaint...");
 try{const d=await api("/api/complaints",{method:"POST",body:JSON.stringify(data)});feedback("complaint-feedback",`${d.message} Ticket #${d.complaint_id}.`,"success");savedForm.reset();await loadDashboard();}
 catch(err){feedback("complaint-feedback",err.message,"error");}
});
const runBtn=$("run-scheduler");
const algorithmSelect=$("algorithm"), quantumWrap=$("quantum-wrap"), quantumInput=$("quantum");
function updateQuantumVisibility(){if(quantumWrap && algorithmSelect){const isRR=algorithmSelect.value==="RR";quantumWrap.hidden=!isRR;if(quantumInput)quantumInput.disabled=!isRR;}}
if(algorithmSelect){algorithmSelect.addEventListener("change",updateQuantumVisibility);updateQuantumVisibility();}
if(runBtn)runBtn.addEventListener("click",async()=>{
 feedback("schedule-message","Running scheduling simulation...");
 try{const selectedAlgorithm=$("algorithm").value;const payload={algorithm:selectedAlgorithm};if(selectedAlgorithm==="RR")payload.quantum=Number($("quantum").value||2);const data=await api("/api/schedule",{method:"POST",body:JSON.stringify(payload)});
 if(!data.results.length){feedback("schedule-message",data.message);$("gantt").innerHTML="";$("schedule-body").innerHTML="";return;}
 feedback("schedule-message",`${data.algorithm} simulation completed for ${data.results.length} delivery process(es).`);
 const timeline=data.gantt.map(g=>`<div class="gantt-segment" style="flex:${Math.max(1,g.end-g.start)} 1 0"><div class="gantt-block"><b>${esc(g.pid)}</b><span>${g.end-g.start} unit(s)</span></div><div class="gantt-ticks"><span>${g.start}</span><span>${g.end}</span></div></div>`).join("");
 $("gantt").innerHTML=`<div class="gantt-caption">Execution order (left to right)</div><div class="gantt-track">${timeline}</div><p class="gantt-note">P# identifies a refill order. Repeated blocks mean a process was preempted; gaps in time are shown when the CPU is idle.</p>`;
 $("schedule-summary").innerHTML=`<div class="stat-card"><span>Average waiting</span><strong>${data.average_waiting}</strong></div><div class="stat-card"><span>Average turnaround</span><strong>${data.average_turnaround}</strong></div>`;
 $("schedule-body").innerHTML=data.results.map(r=>`<tr><td>${esc(r.pid)}</td><td>${r.arrival}</td><td>${r.burst}</td><td>${["","High","Normal","Low"][r.priority]}</td><td>${r.waiting}</td><td>${r.turnaround}</td><td>${r.completion}</td></tr>`).join("");
 toast("Scheduling completed");await loadDashboard();
 }catch(e){feedback("schedule-message",e.message,"error");}
});
if($("refresh-btn"))$("refresh-btn").addEventListener("click",loadDashboard);
loadDashboard();