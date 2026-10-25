from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, KeepTogether
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
import sys

FD = "/usr/share/fonts/truetype/dejavu/"
pdfmetrics.registerFont(TTFont("DV", FD + "DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont("DVB", FD + "DejaVuSans-Bold.ttf"))
pdfmetrics.registerFont(TTFont("DVM", FD + "DejaVuSansMono.ttf"))
pdfmetrics.registerFontFamily("DV", normal="DV", bold="DVB", italic="DV", boldItalic="DVB")

ACC = colors.HexColor("#8a4b1f"); INK = colors.HexColor("#222222")
SOFT = colors.HexColor("#f4ede4"); LINE = colors.HexColor("#d8c8b4")
H1 = ParagraphStyle("h1", fontName="DVB", fontSize=20, leading=25, textColor=ACC, spaceAfter=3)
SUB = ParagraphStyle("sub", fontName="DV", fontSize=10, leading=14, textColor=colors.HexColor("#666666"), spaceAfter=8)
H2 = ParagraphStyle("h2", fontName="DVB", fontSize=13.5, leading=18, textColor=ACC, spaceBefore=10, spaceAfter=5, keepWithNext=1)
P = ParagraphStyle("p", fontName="DV", fontSize=10, leading=14, textColor=INK, spaceAfter=4)
NUM = ParagraphStyle("n", fontName="DVB", fontSize=13, leading=16, textColor=colors.white, alignment=1)
TC = ParagraphStyle("tc", fontName="DV", fontSize=9, leading=12, textColor=INK)
TH = ParagraphStyle("th", fontName="DVB", fontSize=9, leading=12, textColor=colors.white)
W = A4[0] - 36 * mm

def m(s): return f'<font name="DVM" size="9">{s}</font>'

def steps(items):
    data = [[Paragraph(str(i), NUM), Paragraph(t, P)] for i, t in enumerate(items, 1)]
    t = Table(data, colWidths=[10 * mm, W - 10 * mm])
    st = [("VALIGN", (0, 0), (-1, -1), "MIDDLE"), ("BACKGROUND", (0, 0), (0, -1), ACC),
          ("LINEBELOW", (0, 0), (-1, -1), 2, colors.white),
          ("TOPPADDING", (0, 0), (-1, -1), 4), ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
          ("LEFTPADDING", (1, 0), (1, -1), 8), ("BACKGROUND", (1, 0), (1, -1), SOFT)]
    t.setStyle(TableStyle(st)); return t

def box(text, bg="#fff6e0", border="#e8c46a"):
    t = Table([[Paragraph(text, P)]], colWidths=[W])
    t.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, -1), colors.HexColor(bg)),
                           ("BOX", (0, 0), (-1, -1), 0.8, colors.HexColor(border)),
                           ("TOPPADDING", (0, 0), (-1, -1), 6), ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
                           ("LEFTPADDING", (0, 0), (-1, -1), 8), ("RIGHTPADDING", (0, 0), (-1, -1), 8)]))
    return KeepTogether([Spacer(1, 3), t, Spacer(1, 5)])

def table(rows, widths):
    data = [[Paragraph(c, TH) for c in rows[0]]] + [[Paragraph(c, TC) for c in r] for r in rows[1:]]
    t = Table(data, colWidths=widths, repeatRows=1)
    t.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, 0), ACC),
        ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, SOFT]),
        ("GRID", (0, 0), (-1, -1), 0.4, LINE), ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("TOPPADDING", (0, 0), (-1, -1), 4), ("BOTTOMPADDING", (0, 0), (-1, -1), 4)]))
    return t

def ul(items): return [Paragraph(i, P, bulletText="•") for i in items]

T = {}
T["en"] = dict(
    file="Kenshi_Online_Guide_EN.pdf",
    title="Kenshi Online — Multiplayer guide",
    sub="Co-op multiplayer mod for Kenshi 1.0.68 (Steam) · up to 16 players · install, host and join",
    req_h="1. Requirements",
    req=["Windows 10/11 64-bit and <b>Kenshi 1.0.68 (Steam)</b>. The version is shown in the bottom-right corner of the main menu.",
         "<b>Every player</b> installs the same version of the mod.",
         "Close Kenshi before installing."],
    inst_h="2. Install (one click)",
    inst=["Download <b>KenshiOnline_Setup.exe</b> and double-click it. Accept the Windows admin prompt (<b>Yes</b>).",
          "If the blue “Windows protected your PC” screen appears, click <b>More info</b> → <b>Run anyway</b> (the installer is not code-signed).",
          "Choose your language. The installer finds the Kenshi folder by itself; if not, click <b>Browse</b> and pick the folder that contains <b>kenshi_x64.exe</b>.",
          "Click <b>Install</b> and then <b>Finish</b>. That's it."],
    inst_note="To uninstall: Windows Settings → Apps → <b>Kenshi Online</b> → Uninstall. Your original files are backed up in <b>Kenshi\\KenshiMP_backup</b>.",
    host_h="3. Host a game (only the host)",
    host=[f"In the Kenshi folder, double-click {m('KenshiMP.Server.exe')}. A black console opens: that is the server. <b>Keep it open</b> while you play.",
          "If Windows Firewall asks, allow access on private and public networks.",
          f"Open port <b>27800 UDP</b> in your router (Port forwarding / NAT): external and internal port 27800, protocol UDP, destination = your PC's local IPv4 (run {m('ipconfig')}).",
          "Give your friends your <b>public IP</b> (search “what is my ip”). On the same network use your local IP instead."],
    host_note="Can't open ports (CG-NAT, 4G/5G, student housing)? Use a free virtual LAN such as <b>Radmin VPN</b>, <b>ZeroTier</b> or <b>Tailscale</b>: everybody joins the same network and you share the IP that the VPN gives you.",
    join_h="4. Play (everyone, host included)",
    join=["Start Kenshi from Steam (“Play Kenshi”, not the modding tool) and press <b>OK</b> in the launcher.",
          "Click <b>NEW GAME</b>. Use the arrow until the start says <b>Multiplayer</b> and click <b>START</b>.",
          "Create your character (any race: Human, Shek, Hive, Skeleton…) and click <b>CONFIRM</b>.",
          "Wait about <b>90 seconds</b> until the <b>KENSHI ONLINE</b> bar appears at the top of the screen.",
          "Press <b>F1</b> (close Kenshi's tutorial window if it also opens) → <b>JOIN GAME</b>.",
          f"Server IP: the host's IP (the host himself types {m('127.0.0.1')}), port <b>27800</b>, your name → <b>CONNECT</b>.",
          "The top bar shows “CONNECTED as &lt;name&gt; | N players online”. You are moved next to the host and everybody sees the others with their own look."],
    join_note="The game remembers the IP: next time it connects by itself about 90 s after the world loads. Join with F1 only after you are in the world (not from the main menu).",
    keys_h="5. Controls",
    keys=[["Key", "Action"], ["F1", "Multiplayer menu (join / disconnect)"], ["Enter", "Chat"], ["Tab", "Player list"], ["Esc", "Close the current panel"]],
    prob_h="6. Troubleshooting",
    probs=[["Problem", "Fix"],
           ["“Connection failed” / retrying", "Check the IP: the default field may contain another address — delete it and type the host's IP. The server console must be open and port 27800 UDP forwarded (or use a VPN)."],
           ["F1 does nothing", "The mod is still waiting for the world: wait until the KENSHI ONLINE bar appears (~90 s)."],
           ["Only an animal / can't choose race", "You picked the wrong start or have an old version. Reinstall and choose the <b>Multiplayer</b> start."],
           ["Many “Player 1” characters", "That is the “Singleplayer” start of an old version. Reinstall and use the Multiplayer start."],
           ["Game closes when the 2nd player joins / on load", "Old DLL. Reinstall with the latest installer. Do not use the original install.bat or Injector."],
           ["Crash on autosave with two Kenshi on ONE PC", "Both copies share the save folder. Play on separate PCs (testing two copies on one PC is not supported)."]],
    lim_h="7. Known limitations (alpha)",
    lim=["Synced: player positions, characters and their appearance (race, face, hair, body), chat.",
         "Combat damage is not fully synced yet and NPC AI runs locally on each PC, so fights may differ between players.",
         "Pause and game speed are controlled by the server.",
         "Back up your saves (%LOCALAPPDATA%\\kenshi\\save) before playing."],
    credits="Based on <b>Kenshi-Online</b> by The404Studios (MIT license). This build fixes the crashes on load/join, player detection, race selection, remote player spawning and appearance sync.",
)
T["es"] = dict(
    file="Kenshi_Online_Guia_ES.pdf",
    title="Kenshi Online — Guía multijugador",
    sub="Mod cooperativo para Kenshi 1.0.68 (Steam) · hasta 16 jugadores · instalar, alojar y unirse",
    req_h="1. Requisitos",
    req=["Windows 10/11 de 64 bits y <b>Kenshi 1.0.68 (Steam)</b>. La versión sale abajo a la derecha del menú principal.",
         "<b>Todos los jugadores</b> instalan la misma versión del mod.",
         "Cierra Kenshi antes de instalar."],
    inst_h="2. Instalar (un clic)",
    inst=["Descarga <b>KenshiOnline_Setup.exe</b> y haz doble clic. Acepta el aviso de administrador de Windows (<b>Sí</b>).",
          "Si sale la pantalla azul «Windows protegió su PC», pulsa <b>Más información</b> → <b>Ejecutar de todas formas</b> (el instalador no está firmado).",
          "Elige el idioma. El instalador encuentra solo la carpeta de Kenshi; si no, pulsa <b>Examinar</b> y elige la carpeta que contiene <b>kenshi_x64.exe</b>.",
          "Pulsa <b>Instalar</b> y luego <b>Terminar</b>. Ya está."],
    inst_note="Para desinstalar: Configuración de Windows → Aplicaciones → <b>Kenshi Online</b> → Desinstalar. Tus archivos originales quedan copiados en <b>Kenshi\\KenshiMP_backup</b>.",
    host_h="3. Crear partida (solo el anfitrión)",
    host=[f"En la carpeta de Kenshi, haz doble clic en {m('KenshiMP.Server.exe')}. Se abre una consola negra: es el servidor. <b>No la cierres</b> mientras jugáis.",
          "Si el firewall de Windows pregunta, permite el acceso en redes privadas y públicas.",
          f"Abre el puerto <b>27800 UDP</b> en el router (Redirección de puertos / NAT): puerto externo e interno 27800, protocolo UDP, destino = la IPv4 local de tu PC (comando {m('ipconfig')}).",
          "Pasa a tus amigos tu <b>IP pública</b> (busca «cuál es mi ip»). Si estáis en la misma red, usa la IP local."],
    host_note="¿No puedes abrir puertos (CG-NAT, 4G/5G, residencias)? Usa una red virtual gratuita como <b>Radmin VPN</b>, <b>ZeroTier</b> o <b>Tailscale</b>: entráis todos en la misma red y compartes la IP que te da la VPN.",
    join_h="4. Jugar (todos, también el anfitrión)",
    join=["Abre Kenshi desde Steam («Jugar a Kenshi», no la herramienta de mods) y pulsa <b>OK</b> en el lanzador.",
          "Pulsa <b>NUEVO JUEGO</b>. Con la flecha elige el inicio <b>Multiplayer</b> y pulsa <b>EMPEZAR</b>.",
          "Crea tu personaje (cualquier raza: Humano, Shek, Enjambre, Esqueleto…) y pulsa <b>CONFIRMAR</b>.",
          "Espera unos <b>90 segundos</b> hasta que salga arriba la barra <b>KENSHI ONLINE</b>.",
          "Pulsa <b>F1</b> (si también se abre la ventana de tutoriales de Kenshi, ciérrala) → <b>JOIN GAME</b>.",
          f"Server IP: la IP del anfitrión (el propio anfitrión escribe {m('127.0.0.1')}), puerto <b>27800</b>, tu nombre → <b>CONNECT</b>.",
          "Arriba pone «CONNECTED as &lt;nombre&gt; | N players online». Apareces junto al anfitrión y cada uno ve a los demás con su propio aspecto."],
    join_note="El juego recuerda la IP: la próxima vez se conecta solo unos 90 s después de cargar el mundo. Conéctate con F1 solo cuando ya estés dentro del mundo (no desde el menú principal).",
    keys_h="5. Controles",
    keys=[["Tecla", "Acción"], ["F1", "Menú multijugador (unirse / desconectar)"], ["Enter", "Chat"], ["Tab", "Lista de jugadores"], ["Esc", "Cerrar el panel actual"]],
    prob_h="6. Problemas frecuentes",
    probs=[["Problema", "Solución"],
           ["«Connection failed» / reintentando", "Revisa la IP: el campo puede traer otra dirección; bórrala y escribe la del anfitrión. La consola del servidor tiene que estar abierta y el puerto 27800 UDP abierto (o usad una VPN)."],
           ["F1 no hace nada", "El mod aún espera al mundo: espera a que salga la barra KENSHI ONLINE (~90 s)."],
           ["Solo un animal / no deja elegir raza", "Inicio equivocado o versión antigua. Reinstala y elige el inicio <b>Multiplayer</b>."],
           ["Muchos personajes «Player 1»", "Es el inicio «Singleplayer» de una versión antigua. Reinstala y usa el inicio Multiplayer."],
           ["Se cierra al entrar el 2º jugador / al cargar", "DLL antigua. Reinstala con el instalador nuevo. No uses el install.bat ni el Injector originales."],
           ["Se cierra al autoguardar con dos Kenshi en UN PC", "Las dos copias comparten la carpeta de guardado. Jugad en PCs distintos (probar dos copias en un PC no está soportado)."]],
    lim_h="7. Limitaciones conocidas (alfa)",
    lim=["Se sincronizan: posiciones, personajes y su aspecto (raza, cara, pelo, cuerpo) y el chat.",
         "El daño en combate aún no se sincroniza del todo y la IA de los NPC es local en cada PC, así que las peleas pueden verse distintas.",
         "La pausa y la velocidad del juego las controla el servidor.",
         "Haz copia de tus partidas (%LOCALAPPDATA%\\kenshi\\save) antes de jugar."],
    credits="Basado en <b>Kenshi-Online</b> de The404Studios (licencia MIT). Esta versión corrige los cierres al cargar y al unirse, la detección del jugador, la elección de raza, la aparición del otro jugador y la sincronización de aspecto.",
)
T["ru"] = dict(
    file="Kenshi_Online_Guide_RU.pdf",
    title="Kenshi Online — руководство по мультиплееру",
    sub="Кооперативный мод для Kenshi 1.0.68 (Steam) · до 16 игроков · установка, хостинг и подключение",
    req_h="1. Требования",
    req=["Windows 10/11 (64-бит) и <b>Kenshi 1.0.68 (Steam)</b>. Версия указана в правом нижнем углу главного меню.",
         "<b>Каждый игрок</b> устанавливает одну и ту же версию мода.",
         "Перед установкой закройте Kenshi."],
    inst_h="2. Установка (в один клик)",
    inst=["Скачайте <b>KenshiOnline_Setup.exe</b> и запустите его двойным щелчком. Подтвердите запрос администратора Windows (<b>Да</b>).",
          "Если появится синее окно «Система Windows защитила ваш компьютер», нажмите <b>Подробнее</b> → <b>Выполнить в любом случае</b> (установщик не подписан).",
          "Выберите язык. Установщик сам находит папку Kenshi; если нет — нажмите <b>Обзор</b> и выберите папку, в которой лежит <b>kenshi_x64.exe</b>.",
          "Нажмите <b>Установить</b>, затем <b>Готово</b>. Всё."],
    inst_note="Удаление: Параметры Windows → Приложения → <b>Kenshi Online</b> → Удалить. Исходные файлы сохранены в <b>Kenshi\\KenshiMP_backup</b>.",
    host_h="3. Создать игру (только хост)",
    host=[f"В папке Kenshi запустите {m('KenshiMP.Server.exe')}. Откроется чёрная консоль — это сервер. <b>Не закрывайте её</b> во время игры.",
          "Если брандмауэр Windows спросит, разрешите доступ в частных и общественных сетях.",
          f"Откройте порт <b>27800 UDP</b> в роутере (проброс портов / NAT): внешний и внутренний порт 27800, протокол UDP, адрес = локальный IPv4 вашего ПК (команда {m('ipconfig')}).",
          "Сообщите друзьям ваш <b>внешний IP</b> (поиск «мой ip»). В одной локальной сети используйте локальный IP."],
    host_note="Не получается открыть порт (CG-NAT, 4G/5G, общежитие)? Используйте бесплатную виртуальную сеть: <b>Radmin VPN</b>, <b>ZeroTier</b> или <b>Tailscale</b>. Все подключаются к одной сети, а вы сообщаете IP, выданный VPN.",
    join_h="4. Игра (все, включая хоста)",
    join=["Запустите Kenshi через Steam («Играть в Kenshi», не инструмент модов) и нажмите <b>OK</b> в лаунчере.",
          "Нажмите <b>НОВАЯ ИГРА</b>. Стрелкой выберите старт <b>Multiplayer</b> и нажмите <b>НАЧАТЬ</b>.",
          "Создайте персонажа (любая раса: человек, шек, улей, скелет…) и нажмите <b>ПОДТВЕРДИТЬ</b>.",
          "Подождите около <b>90 секунд</b>, пока вверху экрана не появится полоса <b>KENSHI ONLINE</b>.",
          "Нажмите <b>F1</b> (если откроется окно обучения Kenshi — закройте его) → <b>JOIN GAME</b>.",
          f"Server IP: IP хоста (сам хост вводит {m('127.0.0.1')}), порт <b>27800</b>, ваше имя → <b>CONNECT</b>.",
          "Вверху появится «CONNECTED as &lt;имя&gt; | N players online». Вас перенесёт к хосту, и каждый увидит остальных с их собственной внешностью."],
    join_note="Игра запоминает IP: в следующий раз подключение произойдёт само примерно через 90 с после загрузки мира. Подключайтесь через F1 только находясь в мире (не из главного меню).",
    keys_h="5. Управление",
    keys=[["Клавиша", "Действие"], ["F1", "Меню мультиплеера (подключиться / отключиться)"], ["Enter", "Чат"], ["Tab", "Список игроков"], ["Esc", "Закрыть текущую панель"]],
    prob_h="6. Частые проблемы",
    probs=[["Проблема", "Решение"],
           ["«Connection failed» / повтор", "Проверьте IP: в поле может быть другой адрес — сотрите его и введите IP хоста. Консоль сервера должна быть открыта, а порт 27800 UDP проброшен (или используйте VPN)."],
           ["F1 ничего не делает", "Мод ещё ждёт загрузки мира: подождите полосу KENSHI ONLINE (~90 с)."],
           ["Только животное / нельзя выбрать расу", "Неверный старт или старая версия. Переустановите и выберите старт <b>Multiplayer</b>."],
           ["Много персонажей «Player 1»", "Это старт «Singleplayer» старой версии. Переустановите и используйте старт Multiplayer."],
           ["Игра закрывается при входе 2-го игрока / при загрузке", "Старая DLL. Переустановите последней версией установщика. Не используйте оригинальные install.bat и Injector."],
           ["Вылет при автосохранении с двумя Kenshi на ОДНОМ ПК", "Обе копии используют одну папку сохранений. Играйте на разных ПК (две копии на одном ПК не поддерживаются)."]],
    lim_h="7. Известные ограничения (альфа)",
    lim=["Синхронизируются: позиции, персонажи и их внешность (раса, лицо, волосы, тело), чат.",
         "Урон в бою синхронизируется ещё не полностью, а ИИ NPC работает локально на каждом ПК, поэтому бои могут выглядеть по-разному.",
         "Пауза и скорость игры управляются сервером.",
         "Сделайте резервную копию сохранений (%LOCALAPPDATA%\\kenshi\\save) перед игрой."],
    credits="Основано на <b>Kenshi-Online</b> от The404Studios (лицензия MIT). Эта сборка исправляет вылеты при загрузке и подключении, определение игрока, выбор расы, появление другого игрока и синхронизацию внешности.",
)

def build(lang, outdir):
    t = T[lang]
    s = [Paragraph(t["title"], H1), Paragraph(t["sub"], SUB)]
    s.append(Paragraph(t["req_h"], H2)); s += ul(t["req"])
    s.append(Paragraph(t["inst_h"], H2)); s.append(steps(t["inst"])); s.append(box(t["inst_note"], "#f4ede4", "#d8c8b4"))
    s.append(Paragraph(t["host_h"], H2)); s.append(steps(t["host"])); s.append(box(t["host_note"]))
    s.append(Paragraph(t["join_h"], H2)); s.append(steps(t["join"])); s.append(box(t["join_note"], "#fde8e4", "#d9826f"))
    s.append(Paragraph(t["keys_h"], H2)); s.append(table(t["keys"], [30 * mm, W - 30 * mm]))
    s.append(Paragraph(t["prob_h"], H2)); s.append(table(t["probs"], [52 * mm, W - 52 * mm]))
    s.append(Paragraph(t["lim_h"], H2)); s += ul(t["lim"])
    s.append(Spacer(1, 6)); s.append(Paragraph(t["credits"], SUB))
    def footer(c, d):
        c.saveState(); c.setFont("DV", 8); c.setFillColor(colors.HexColor("#888888"))
        c.drawString(18 * mm, 10 * mm, t["title"]); c.drawRightString(A4[0] - 18 * mm, 10 * mm, str(d.page))
        c.restoreState()
    doc = SimpleDocTemplate(f"{outdir}/{t['file']}", pagesize=A4, leftMargin=18 * mm, rightMargin=18 * mm,
                            topMargin=15 * mm, bottomMargin=17 * mm, title=t["title"], author="Kenshi Online contributors",
                            creator="Kenshi Online", subject=t["sub"])
    doc.build(s, onFirstPage=footer, onLaterPages=footer)

if __name__ == "__main__":
    out = sys.argv[1]
    for l in ("en", "es", "ru"): build(l, out)
    print("ok")
