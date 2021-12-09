# web_hw2

請設計一對client--server的OX棋下棋程式，具有下列幾個特性：

1. 允許至少2個client同時登錄至server。     

2. client的使用者可以列出已登入的使用者名單。

3. client的使用者可以選擇要跟哪一個使用者下棋，並請求對方的同意。

4. 若對方同意後，開始進入棋局。

5. 雙方可輪流下棋，直到分出勝負或平手。

6. 登入的使用者可選擇登出。



ps.

1. 前述未詳盡指定的規格，同學可以用對自己最方便的方式自行定義。

2. 指令或執行畫面(例如棋盤...)，同學可自行定義，畫面可用純ASCII字元顯示。

3. 為方便起見，使用者的帳號與密碼可事先內建在server中，不用提供註冊功能。

-----------------------------------------------------------------------
帳號註冊在account裡 格式 : "帳號_密碼" 可更改帳號＆密碼

帳號 ac1 ac2 ac3 ac4 ac5

密碼 pw1 pw2 pw3 pw4 pw5

-----------------------------------------------------------------------
使用./client登入 輸入帳號&密碼

用ls指令查看線上玩家和他的fd

用@fd 發送邀請 等待回應

收到邀請: 輸入OK enter 再按enter

開始下棋: 輸入#0~#8下棋

登出: logout

其他輸入: 聊天
