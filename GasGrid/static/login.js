let selectedRole = "user";
document.querySelectorAll(".tab").forEach(btn => btn.addEventListener("click", () => {
  selectedRole = btn.dataset.role;
  document.querySelectorAll(".tab").forEach(b => b.classList.toggle("active", b === btn));
}));
async function send(url, body) {
  const r = await fetch(url,{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(body)});
  const d = await r.json();
  if(!r.ok) throw new Error(d.error || "Request failed.");
  return d;
}
document.getElementById("login-form").addEventListener("submit",async e=>{
  e.preventDefault(); const f=e.currentTarget; const d=Object.fromEntries(new FormData(f).entries()); d.role=selectedRole;
  const out=document.getElementById("login-feedback"); out.textContent="Signing in..."; out.className="feedback";
  try { await send("/api/login",d); location.href="/"; } catch(err){out.textContent=err.message;out.className="feedback error";}
});
document.getElementById("register-form").addEventListener("submit",async e=>{
  e.preventDefault(); const f=e.currentTarget; const d=Object.fromEntries(new FormData(f).entries());
  const out=document.getElementById("register-feedback"); out.textContent="Creating account..."; out.className="feedback";
  try { const result=await send("/api/register",d); out.textContent=result.message;out.className="feedback success";f.reset(); }
  catch(err){out.textContent=err.message;out.className="feedback error";}
});

// Demo password recovery uses a short-lived one-time code; production should use verified SMS/email.
const recoveryPanel = document.getElementById("recovery-panel");
document.getElementById("show-recovery").addEventListener("click", () => {
  recoveryPanel.hidden = !recoveryPanel.hidden;
  if (!recoveryPanel.hidden) recoveryPanel.scrollIntoView({behavior:"smooth", block:"nearest"});
});
document.getElementById("recovery-request-form").addEventListener("submit", async e => {
  e.preventDefault();
  const form = e.currentTarget;
  const phone = form.elements.phone.value.trim();
  const out = document.getElementById("recovery-request-feedback");
  const codeBox = document.getElementById("demo-code-box");
  out.textContent = "Generating recovery code..."; out.className = "feedback"; codeBox.hidden = true;
  try {
    const result = await send("/api/forgot-password", {phone, role:selectedRole});
    out.textContent = result.message; out.className = "feedback success";
    if (result.demo_code) {
      codeBox.hidden = false;
      codeBox.textContent = `Demo recovery code: ${result.demo_code} (valid for 10 minutes)`;
      const resetForm = document.getElementById("recovery-reset-form");
      resetForm.dataset.phone = phone;
    }
  } catch(err) { out.textContent = err.message; out.className = "feedback error"; }
});
document.getElementById("recovery-reset-form").addEventListener("submit", async e => {
  e.preventDefault();
  const form = e.currentTarget;
  const requestForm = document.getElementById("recovery-request-form");
  const phone = form.dataset.phone || requestForm.elements.phone.value.trim();
  const out = document.getElementById("recovery-reset-feedback");
  out.textContent = "Resetting password..."; out.className = "feedback";
  try {
    const result = await send("/api/reset-password", {phone, role:selectedRole, code:form.elements.code.value.trim(), new_password:form.elements.new_password.value});
    out.textContent = result.message; out.className = "feedback success"; form.reset();
    document.getElementById("demo-code-box").hidden = true;
  } catch(err) { out.textContent = err.message; out.className = "feedback error"; }
});
