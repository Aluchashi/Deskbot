#pragma once

const char DASH_HTML[] PROGMEM = R"DASH(<!DOCTYPE html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>Desk Bot</title>
<style>
:root{--bg:#f5f5f4;--card:#ffffff;--tx:#18181b;--mut:#8a8a93;--line:#ececea;--soft:#f3f3f2;--ac:#3b6cf6;--acx:#ffffff;--bad:#e5484d}
@media(prefers-color-scheme:dark){:root{--bg:#0c0c0e;--card:#151517;--tx:#f4f4f5;--mut:#85858d;--line:#242428;--soft:#1e1e22;--ac:#5b86ff;--acx:#0c0c0e;--bad:#ff6369}}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;padding:20px 16px 40px;background:var(--bg);color:var(--tx);font-family:system-ui,-apple-system,"Segoe UI",sans-serif;-webkit-font-smoothing:antialiased}
.w{max-width:440px;margin:0 auto}
.head{display:flex;justify-content:space-between;align-items:flex-end;padding:4px 4px 18px}
.head h1{margin:0;font-size:13px;font-weight:600;letter-spacing:.12em;text-transform:uppercase;color:var(--mut)}
.clock{font-size:40px;font-weight:600;letter-spacing:-.02em;font-variant-numeric:tabular-nums;line-height:1.1;margin-top:4px}
.wx{font-size:13px;color:var(--mut);margin-top:4px}
.dot{display:inline-block;width:7px;height:7px;border-radius:50%;background:#30a46c;margin-right:6px}
.dot.off{background:var(--bad)}
.card{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:18px;margin-bottom:12px}
h2{margin:0 0 14px;font-size:12px;font-weight:600;letter-spacing:.1em;text-transform:uppercase;color:var(--mut)}
button{font:inherit;font-size:14px;font-weight:500;border:0;border-radius:12px;padding:11px 14px;background:var(--soft);color:var(--tx);cursor:pointer;transition:transform .08s,opacity .15s}
button:active{transform:scale(.97)}
button.p{background:var(--ac);color:var(--acx)}
button.on{background:var(--ac);color:var(--acx)}
.pills{display:flex;flex-wrap:wrap;gap:8px}
.pills button{padding:9px 14px;border-radius:999px;font-size:13px}
.row{display:flex;gap:8px;align-items:center;margin-top:12px}
.row>button{flex:1}
input[type=text],input[type=time],input[type=number]{width:100%;font:inherit;font-size:15px;color:var(--tx);background:var(--soft);border:1px solid transparent;border-radius:12px;padding:12px 14px;outline:0}
input:focus{border-color:var(--ac)}
.stack{display:flex;flex-direction:column;gap:8px}
.vc{display:flex;flex-direction:column;align-items:center;text-align:center}
#mb{width:72px;height:72px;border-radius:50%;padding:0;display:flex;align-items:center;justify-content:center;background:var(--ac);color:var(--acx)}
#mb svg{width:28px;height:28px}
#mb.live{animation:pl 1.2s infinite}
@keyframes pl{0%{box-shadow:0 0 0 0 rgba(91,134,255,.55)}100%{box-shadow:0 0 0 24px rgba(91,134,255,0)}}
#hd{margin-top:14px;font-size:16px;font-weight:500;min-height:22px}
#rs{margin-top:4px;font-size:13px;color:var(--mut);min-height:18px}
details{margin-top:14px;font-size:12px;color:var(--mut);width:100%}
summary{cursor:pointer;text-align:center}
details p{margin:8px 0 0;line-height:1.8;text-align:left}
.ringbox{position:relative;width:190px;height:190px;margin:2px auto 6px}
.ringbox svg{width:100%;height:100%;transform:rotate(-90deg)}
.ringbox circle{fill:none;stroke-width:6;stroke-linecap:round}
.rbg{stroke:var(--soft)}
.rfg{stroke:var(--ac);stroke-dasharray:327;stroke-dashoffset:327;transition:stroke-dashoffset .9s linear}
.rtx{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center}
.big{font-size:44px;font-weight:600;letter-spacing:-.02em;font-variant-numeric:tabular-nums}
.sub{font-size:12px;color:var(--mut);margin-top:2px}
.num{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:16px}
.num label{display:block;font-size:11px;color:var(--mut);margin-bottom:5px}
.num input{padding:10px;text-align:center}
.al{padding:14px 0;border-top:1px solid var(--line)}
.al:first-child{border-top:0;padding-top:0}
.top{display:flex;align-items:center;gap:12px}
.top input[type=time]{flex:1;font-size:20px;font-weight:600}
.sw{position:relative;width:46px;height:28px;flex:none}
.sw input{opacity:0;width:100%;height:100%;margin:0;position:absolute;z-index:2;cursor:pointer}
.sw span{position:absolute;inset:0;background:var(--soft);border-radius:999px;transition:.2s}
.sw span:after{content:"";position:absolute;left:3px;top:3px;width:22px;height:22px;border-radius:50%;background:var(--mut);transition:.2s}
.sw input:checked+span{background:var(--ac)}
.sw input:checked+span:after{transform:translateX(18px);background:var(--acx)}
.chips{display:flex;gap:6px;margin-top:12px}
.chip{flex:1;text-align:center;padding:9px 0;border-radius:10px;background:var(--soft);font-size:12px;font-weight:500;color:var(--mut);cursor:pointer;user-select:none}
.chip.on{background:var(--ac);color:var(--acx)}
.hint{font-size:11px;color:var(--mut);margin-top:10px}
.ring{display:none;border-color:var(--bad)}
.ring.show{display:block}
.ring h2{color:var(--bad)}
#tt{position:fixed;left:50%;bottom:24px;transform:translate(-50%,20px);background:var(--tx);color:var(--bg);padding:10px 18px;border-radius:999px;font-size:13px;opacity:0;pointer-events:none;transition:.25s}
#tt.show{opacity:1;transform:translate(-50%,0)}
table.cm{width:100%;border-collapse:collapse;font-size:13px}
table.cm th{text-align:left;font-size:11px;font-weight:600;letter-spacing:.08em;text-transform:uppercase;color:var(--mut);padding:0 0 8px}
table.cm td{padding:9px 0;border-top:1px solid var(--line);vertical-align:top;line-height:1.5}
table.cm td:first-child{width:52%;padding-right:12px}
table.cm tr.g td{border-top:0;padding:16px 0 4px;font-size:11px;font-weight:600;letter-spacing:.1em;text-transform:uppercase;color:var(--ac)}
table.cm tr.g:first-of-type td{padding-top:4px}
table.cm code{display:inline-block;background:var(--soft);border-radius:6px;padding:2px 7px;margin:1px 2px 1px 0;font-family:inherit;font-size:12px}
</style></head><body><div class="w">
<div class="head"><div><h1><span class="dot" id="dt"></span>Desk Bot</h1><div class="clock" id="clk">--:--:--</div><div class="wx" id="wx"></div></div></div>

<div class="card ring" id="ring"><h2>Alarm ringing</h2><div class="row" style="margin-top:0"><button class="p" onclick="stopA(0)">Stop</button><button onclick="stopA(1)">Snooze 5 min</button></div></div>

<div class="card"><h2>Voice</h2><div class="vc">
<button id="mb" onclick="toggleMic()" aria-label="Voice command"><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="9" y="2" width="6" height="12" rx="3"/><path d="M5 11a7 7 0 0 0 14 0"/><path d="M12 18v4"/></svg></button>
<div id="hd">Tap and speak</div><div id="rs"></div>
<details><summary>What can I say?</summary>
<p><b>Mood:</b> happy, sad, angry, sleepy, love<br>
<b>Page:</b> show weather, clock, world clock, forecast, eyes<br>
<b>Pomodoro:</b> start pomodoro, pause, resume, reset, skip<br>
<b>Alarm:</b> alarm in 10 minutes, alarm at 7:30 am, stop, snooze<br>
<b>Display:</b> dim, brightness high<br>
<b>Message:</b> say hello</p></details>
</div></div>

<div class="card"><h2>Mood</h2><div class="pills" id="moods"></div></div>

<div class="card"><h2>Message</h2>
<div class="stack"><input type="text" id="mt" placeholder="Title" maxlength="40"><input type="text" id="mm" placeholder="Message (English only)" maxlength="80"></div>
<div class="row"><button class="p" onclick="sendMsg()">Show on display</button></div></div>

<div class="card"><h2>Pomodoro</h2>
<div class="ringbox"><svg viewBox="0 0 120 120"><circle class="rbg" cx="60" cy="60" r="52"/><circle class="rfg" id="rf" cx="60" cy="60" r="52"/></svg>
<div class="rtx"><div class="big" id="pt">25:00</div><div class="sub" id="pp">Ready</div></div></div>
<div class="row"><button class="p" id="pb" onclick="pomo('toggle')">Start</button><button onclick="pomo('reset')">Reset</button><button onclick="pomo('skip')">Skip</button></div>
<div class="num"><div><label>Focus</label><input type="number" id="pw" min="1" max="120"></div><div><label>Break</label><input type="number" id="pbk" min="1" max="60"></div><div><label>Long</label><input type="number" id="pl" min="1" max="120"></div><div><label>Rounds</label><input type="number" id="pr" min="2" max="6"></div></div>
<div class="row"><button onclick="saveP()">Save settings</button></div></div>

<div class="card"><h2>Alarms</h2><div id="alarms"></div></div>

<div class="card"><h2>Show page</h2><div class="pills" id="pages"></div></div>
<div class="card"><h2>Voice commands</h2>
<table class="cm">
<tr><th>Say</th><th>What happens</th></tr>

<tr class="g"><td colspan="2">Mood</td></tr>
<tr><td><code>happy</code> <code>smile</code></td><td>Happy eyes</td></tr>
<tr><td><code>sad</code></td><td>Sad eyes</td></tr>
<tr><td><code>angry</code> <code>mad</code></td><td>Angry eyes</td></tr>
<tr><td><code>sleepy</code> <code>tired</code></td><td>Sleepy eyes</td></tr>
<tr><td><code>surprised</code></td><td>Surprised eyes</td></tr>
<tr><td><code>excited</code></td><td>Excited eyes</td></tr>
<tr><td><code>love</code></td><td>Love eyes</td></tr>
<tr><td><code>suspicious</code></td><td>Suspicious eyes</td></tr>
<tr><td><code>normal</code></td><td>Normal eyes</td></tr>

<tr class="g"><td colspan="2">Pages</td></tr>
<tr><td><code>eyes</code> <code>face</code></td><td>Show the eyes page</td></tr>
<tr><td><code>clock</code> <code>time</code></td><td>Show the clock page</td></tr>
<tr><td><code>weather</code></td><td>Show the weather page</td></tr>
<tr><td><code>forecast</code></td><td>Show the 3-day forecast</td></tr>
<tr><td><code>world clock</code></td><td>Show the world clock</td></tr>
<tr><td><code>focus</code></td><td>Show the focus timer page</td></tr>

<tr class="g"><td colspan="2">Focus timer</td></tr>
<tr><td><code>start</code> <code>begin</code></td><td>Start the focus timer</td></tr>
<tr><td><code>pause focus</code> <code>stop focus</code></td><td>Pause the focus timer</td></tr>
<tr><td><code>resume</code></td><td>Continue a paused timer</td></tr>
<tr><td><code>skip</code></td><td>Jump to the next phase (break or focus)</td></tr>
<tr><td><code>reset</code></td><td>Reset the focus timer</td></tr>

<tr class="g"><td colspan="2">Alarm</td></tr>
<tr><td><code>alarm in 10 minutes</code></td><td>Sets a one-time alarm 10 minutes from now</td></tr>
<tr><td><code>alarm at 7:30 am</code></td><td>Sets a one-time alarm for that time</td></tr>
<tr><td><code>stop</code> <code>dismiss</code> <code>silence</code> <code>quiet</code></td><td>Stops a ringing alarm</td></tr>
<tr><td><code>snooze</code></td><td>Snoozes the alarm for 5 minutes</td></tr>

<tr class="g"><td colspan="2">Display</td></tr>
<tr><td><code>brightness high</code></td><td>Bright screen</td></tr>
<tr><td><code>dim</code> <code>dark</code> <code>low brightness</code></td><td>Dim screen</td></tr>

<tr class="g"><td colspan="2">Message</td></tr>
<tr><td><code>say hello</code></td><td>Shows "hello" on the display (works with any words after "say")</td></tr>
</table></div>
</div><div id="tt"></div><script>
const $=i=>document.getElementById(i);
const MOODS=["Normal","Happy","Surprised","Sleepy","Angry","Sad","Excited","Love","Suspicious"];
const PAGES=["Eyes","Clock","Weather","Forecast","World","Pomodoro"];
const DN=["S","M","T","W","T","F","S"];
const pad=n=>String(n).padStart(2,"0");
const fmt=s=>pad(Math.floor(s/60))+":"+pad(s%60);
const api=p=>fetch(p).then(r=>r.text()).catch(()=>"");
const toast=t=>{const e=$("tt");e.textContent=t;e.classList.add("show");setTimeout(()=>e.classList.remove("show"),1400)};
let init=false,lastS=null;
const build=S=>{
  $("moods").innerHTML=MOODS.map((m,i)=>`<button id="m${i}" onclick="setMood(${i})">${m}</button>`).join("");
  $("pages").innerHTML=PAGES.map((m,i)=>`<button onclick="setPage(${i})">${m}</button>`).join("");
  $("alarms").innerHTML=S.alarms.map((a,i)=>{
    let c="";
    for(let k=0;k<7;k++)c+=`<span class="chip${a.d&(1<<k)?" on":""}" onclick="this.classList.toggle('on')">${DN[k]}</span>`;
    return `<div class="al"><div class="top"><input type="time" id="t${i}" value="${pad(a.h)}:${pad(a.m)}"><label class="sw"><input type="checkbox" id="e${i}"${a.e?" checked":""}><span></span></label></div><div class="chips" id="c${i}">${c}</div><div class="row"><button class="p" onclick="saveA(${i})">Save</button><button onclick="testA(${i})">Test ring</button></div><div class="hint">No day selected = ring once</div></div>`;
  }).join("");
  $("pw").value=S.pomo.work;$("pbk").value=S.pomo.brk;$("pl").value=S.pomo.lng;$("pr").value=S.pomo.rounds;
}
const refresh=async()=>{
  let S;
  try{S=await (await fetch("/api/state")).json()}catch(e){$("dt").classList.add("off");return}
  lastS=S;
  $("dt").classList.remove("off");
  if(!init){build(S);init=true}
  $("clk").textContent=S.time;
  $("wx").textContent=S.weather.ok?S.weather.temp+"\u00b0C  "+S.weather.main:"Weather loading...";
  MOODS.forEach((m,i)=>$("m"+i).classList.toggle("on",S.mood===i));
  const p=S.pomo;
  const tot=[p.work,p.brk,p.lng][p.phase]*60;
  const prog=p.started&&tot?Math.min(1,Math.max(0,(tot-p.left)/tot)):0;
  $("rf").style.strokeDashoffset=327*(1-prog);
  $("pt").textContent=fmt(p.left);
  $("pp").textContent=!p.started?"Ready":["Focus","Break","Long break"][p.phase]+(p.running?"":" (paused)")+"  "+p.done+"/"+p.rounds;
  $("pb").textContent=!p.started?"Start":p.running?"Pause":"Resume";
  $("ring").classList.toggle("show",!!S.ringing);
}
const setMood=async i=>{await api("/api/mood?m="+i);refresh()}
const setPage=async i=>{await api("/api/page?p="+i);toast(PAGES[i])}
const sendMsg=async()=>{
  const t=$("mt").value,m=$("mm").value;
  if(!t&&!m)return;
  await api("/notify?title="+encodeURIComponent(t)+"&msg="+encodeURIComponent(m));
  $("mt").value="";$("mm").value="";toast("Sent");
}
const pomo=async a=>{await api("/api/pomo?a="+a);refresh()}
const saveP=async()=>{
  await api("/api/pomo/set?work="+$("pw").value+"&brk="+$("pbk").value+"&lng="+$("pl").value+"&rounds="+$("pr").value);
  toast("Saved");refresh();
}
const saveA=async i=>{
  const hm=($("t"+i).value||"07:00").split(":");
  let d=0;
  $("c"+i).querySelectorAll(".chip").forEach((c,k)=>{if(c.classList.contains("on"))d|=1<<k});
  await api("/api/alarm?i="+i+"&h="+parseInt(hm[0],10)+"&m="+parseInt(hm[1],10)+"&e="+($("e"+i).checked?1:0)+"&d="+d);
  toast("Alarm saved");
}
const testA=async i=>{await api("/api/ring?i="+i);refresh()}
const stopA=async sn=>{await api("/api/stop?snooze="+sn);refresh()}

const NW={zero:0,one:1,two:2,three:3,four:4,five:5,six:6,seven:7,eight:8,nine:9,ten:10,eleven:11,twelve:12,thirteen:13,fourteen:14,fifteen:15,sixteen:16,seventeen:17,eighteen:18,nineteen:19,twenty:20,thirty:30,forty:40,fifty:50,sixty:60};
const NWR=new RegExp("\\b("+Object.keys(NW).join("|")+")\\b","g");
const norm=s=>{
  let t=s.toLowerCase().replace(/a\.m\.?/g,"am").replace(/p\.m\.?/g,"pm").replace(/[.,!?]/g," ");
  t=t.replace(/half an hour/g,"30 minutes").replace(/\b(an|a) (hour|minute)/g,"1 $2");
  t=t.replace(NWR,w=>NW[w]);
  t=t.replace(/\b([2-5]0) (\d)\b/g,(m,a,b)=>String(+a+ +b));
  return t.replace(/\s+/g," ").trim();
};
const MD=[[/\bnormal\b/,0],[/\bhappy\b|\bsmile\b/,1],[/surpris/,2],[/sleep|tired/,3],[/\bangry\b|\bmad\b/,4],[/\bsad\b/,5],[/excite/,6],[/\blove\b/,7],[/suspic/,8]];
const PG=[[/world/,4],[/forecast/,3],[/weather/,2],[/focus/,5],[/clock|time/,1],[/eyes|face/,0]];
const putAlarm=async(h,m)=>{
  const al=(lastS&&lastS.alarms)||[];
  let i=al.findIndex(a=>!a.e);
  if(i<0)i=2;
  await api("/api/alarm?i="+i+"&h="+h+"&m="+m+"&e=1&d=0");
  $("t"+i).value=pad(h)+":"+pad(m);
  $("e"+i).checked=true;
  $("c"+i).querySelectorAll(".chip").forEach(c=>c.classList.remove("on"));
  return "Alarm "+pad(h)+":"+pad(m)+" set (slot "+(i+1)+")";
};
const run=async raw=>{
  const t=norm(raw);
  let m;
  if(/snooze/.test(t)){await stopA(1);return "Snoozed"}
    if(/\b(stop|dismiss|silence|quiet)\b/.test(t)&&!/focus/.test(t)){await stopA(0);return "Stopped"}
  if(/alarm|wake me/.test(t)){
    let mins=0;
    for(const x of t.matchAll(/(\d+)\s*(hour|hr|minute|min)/g))mins+=+x[1]*(x[2][0]=="h"?60:1);
    if(mins>0){
      const c=lastS&&lastS.time&&lastS.time.match(/^(\d+):(\d+)/);
      if(!c)return "Device time not synced yet";
      const tot=(+c[1]*60+ +c[2]+mins)%1440;
      return await putAlarm(Math.floor(tot/60),tot%60);
    }
    m=t.match(/(\d{1,2})(?:[: ](\d{2}))?\s*(am|pm)?/);
    if(m){
      let h=+m[1];const mi=m[2]?+m[2]:0;
      if(m[3]=="pm"&&h<12)h+=12;
      if(m[3]=="am"&&h==12)h=0;
      if(h>23||mi>59)return "Invalid time";
      return await putAlarm(h,mi);
    }
    return "Say a time, e.g. alarm in 10 minutes";
  }
  if(/bright|\bdim\b|dark/.test(t)){
    const low=/\b(low|dim|dark|lower|down|less)\b/.test(t);
    await api("/api/bright?v="+(low?0:1));
    return low?"Dim":"Bright";
  }
  if(/reset/.test(t)){await pomo("reset");return "Pomodoro reset"}
  if(/skip/.test(t)){await pomo("skip");return "Skipped"}
    if(/pause|stop|hold/.test(t)&&/focus|pause/.test(t)){
    if(lastS&&lastS.pomo.running){await pomo("toggle");return "Paused"}
    return "Not running";
  }
  if(/\b(start|resume|begin)\b/.test(t)){
    const p=lastS&&lastS.pomo;
    if(p&&p.running)return "Already running";
    await pomo("toggle");
    return p&&p.started?"Resumed":"Pomodoro started";
  }
  for(const [r,i] of MD)if(r.test(t)){await setMood(i);return "Mood: "+MOODS[i]}
  for(const [r,i] of PG)if(r.test(t)){await setPage(i);return "Page: "+PAGES[i]}
  m=t.match(/^(?:say|message)\s+(.+)/);
  if(m){await api("/notify?title=Voice&msg="+encodeURIComponent(m[1]));return "Message sent"}
  return "Didn't get that";
};

const SR=window.SpeechRecognition||window.webkitSpeechRecognition;
let rec=null,listening=false;
const say=(a,b)=>{$("hd").textContent=a;$("rs").textContent=b||""};
const toggleMic=()=>{
  if(!SR){say("Voice not supported","Use Chrome on Android");return}
  if(listening){rec.stop();return}
  rec=new SR();
  rec.lang="en-US";
  rec.interimResults=true;
  rec.maxAlternatives=1;
  let fin="",err="";
  rec.onstart=()=>{listening=true;$("mb").classList.add("live");say("Listening...","")};
  rec.onresult=e=>{
    let s="";
    for(let i=0;i<e.results.length;i++){s+=e.results[i][0].transcript;if(e.results[i].isFinal)fin=s}
    say(s,"");
  };
  rec.onerror=e=>{
    err=e.error;
    say("Mic error: "+e.error,e.error=="not-allowed"?"Allow the mic and check the Chrome flag":e.error=="network"?"Phone needs internet for speech":"");
  };
  rec.onend=async()=>{
    listening=false;$("mb").classList.remove("live");
    if(fin){const r=await run(fin);say(fin,r)}
    else if(!err)say("Didn't hear anything","Tap and try again");
  };
  rec.start();
};
refresh();setInterval(refresh,1000);
</script></body></html>)DASH";