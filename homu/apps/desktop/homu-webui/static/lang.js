langName = "ja";

lang = {
         logout:                   "ログアウト",
         delete:                   "削除",
         add:                      "追加",
         close:                    "閉じる",
         path:                     "パス",
         hide:                     "消す",
         res_NULL:                 "空",
         res_NET_CONNECT:          "ネットワークへのアクセス",
         res_NET_SERVER:           "ネットワークからのアクセスの受付",
         res_FS_READ:              "ファイル・フォルダの読み込み",
         res_FS_WRITE:             "ファイル・フォルダへの書き込み、作成",
         res_SYS_CHANGE:           "システムへの危険な変更",
         res_HW_FULLCTRL:          "ハードウェアの全ての操作",
         userres_FILE:             "ファイル",
         log_WARNING:              "Homuの内部で警告が発生しました",
         log_ERROR:                "Homuの内部エラーが発生しました",
         log_RULE_VIOLATION:       "ルール違反が発生しました",
         log_MODE_CHANGED:         "動作モードが変更されました",
         mode_SLEEP:               "スリープモード",
         mode_OBSERVE:             "監視モード",
         mode_INVOKE:              "アクセス制御モード",
         notification:             "通知",
         new_notification:         "新しい通知があります",
         tech_info:                "技術的情報",
         mode:                     "モード",
         ex_app:                   "例外のアプリケーション",
         add_exception:            "例外の追加",
         author:                   "開発元",
         description:              "説明",
         url:                      "URL",
         target_app:               "例外として追加するアプリケーション",
         search_not_found:         "見つかりませんでした",
         res_to_be_allowed:        "許可するリソース",
         isNotExApp:               "例外として登録されていないアプリ",
         app:                      "アプリ",
         res:                      "リソース",
         edit_res:                 "リソースの編集",
         allowed_res:              "許可しているリソース",
         user_res:                 "ユーザリソース",
         add_user_res:             "ユーザリソースの追加",
         type:                     "種類",
         user_res_str:             "対象",
         change_mode:              "モードを変更する",
         change:                   "変更する",
         current_mode:             "現在の動作モードは%sです。",
         apply_rules:              "ルールを適用する",
         about_homu:               "Homuついて",
         about_homu_body:          "Homuは<a href='http://www.opensource.org/licenses/mit-license.html'>MITライセンス</a>の下でオープンソースソフトウェアとして公開されており、"       +
                                   "<a href='http://bitbucket.org/seiya/homu'>Bitbucket</a>より"  +
                                   "ソースコードをダウンロードすることができます。",
         err_invalid_request:      "ソフトウェアのバグ: 不正なリクエスト",
         help_mode:                "Homuは以下3つの動作モードのいずれかで動作します。<br>"  +
                                   " <b>スリープモード: </b>Homuを無効化します<br>" +
                                   " <b>監視モード: </b>アプリが許可していないリソースへのアクセスを監視します。アクセスの拒否はしません。<br>" +
                                   " <b>アクセス制御モード: </b>監視モードに加え、許可していないリソースへのアクセスを拒否します。",

         help_user_res_str:        "新しくユーザリソースとして登録するものを選択してください。",
         help_user_res_desc:       "自分にとって分かりやすい説明を入力してください。勿論この説明の内容はHomuの動作に影響しません。",
         help_res_to_be_allowed:   "アプリに許可するリソースを選択してください。アプリはここで選択されていないリソースにはアクセスできません。",
         help_search_target_app:   "新しく例外として追加するアプリを検索窓から検索し、選択してください。アプリの名前で検索することをお勧めします。",

         welcome_1_title:    "Homuへようこそ！",
         welcome_1_body:     "それではHomuでどういうことが出来るのか学びましょう。",
         welcome_2_title:    "Homuで出来ること",
         welcome_2_body:     "Homuを使ってこれら二つのことができます。"      +
                             "<ul>"                                          +
                             "  <li>アプリケーションの出来ることの制限</li>" +
                             "  <li>大切なファイルへのアクセスの制限</li>"   +
                             "</ul>",
         welcome_3_title:    "アプリケーションの出来ることの制限",
         welcome_3_body:     "Homuではアプリケーションが出来ること(アクセスできるもの)を「リソース」と呼んでいます。<br>"          +
                             "リソースにはインターネットへのアクセスやファイルへの書き込みなどがあります。<br>"                    +
                             "Homuを使うと、アプリケーションのアクセスできるリソースを、例外として個別に制限することが出来ます。",
         welcome_4_title:    "大切なファイルへのアクセスの制限",
         welcome_4_body:     "一口に「ファイル」といっても、住所録から可愛い子猫の写真、アプリケーションなど様々なファイルがあります。<br>" +
                             "Homuでは「大切なファイル」を新しいリソース（ユーザリソース）として定義し、<br>"                               +
                             "そのユーザリソースへのアクセス権限がないと読み書きを拒否するようにできます。<br>"                             +
                             "ユーザリソースへのアクセス権限は、例外として個別に付与する必要があります。",
         welcome_5_title:    "Homuがアクセス拒否するとき",
         welcome_5_body:     "原則的に、Homuはアプリケーションのリソースへのアクセスを許可します。<br>"                                          +
                             "Homuによるリソースへのアクセス拒否が行われる場合は、以下二つのどちらかの場合となります。<br>"                      +
                             "<ul>"                                                                                                              +
                             "  <li>ユーザリソースとして定義されているファイルに、それのアクセス権限を持たずして読み書きをしようとした場合</li>" +
                             "  <li>例外として登録されており、アクセスできるリソースが制限されている場合</li>"                                   +
                             "</ul>",
         welcome_6_title:    "最後に",
         welcome_6_body:     "Homuは<a href='http://www.opensource.org/licenses/mit-license.html'>MITライセンス</a>の下でオープンソースソフトウェアとして公開されています。" +
                             "ソースコードは<a href='http://bitbucket.org/seiya/homu'>Bitbucketよりダウンロードできます</a>。",
         welcome_7_title:    "それではHomuを始めましょう",
         welcome_7_body:     "<button onClick='leaveTour()'>Homuを始める</button>",


         __last__: ""
       };
