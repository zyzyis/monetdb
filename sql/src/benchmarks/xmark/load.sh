#!/bin/sh

PWD=`pwd`
Mserver -db sql \
	$MONET_PREFIX/share/Monet/mapi.mil \
	$SQL_PREFIX/share/MonetDB/sqlserver.mil << EOF

proc input( str tab, str filename ) := {
	bulkload(toLower(tab), "$PWD" + "/" + filename, "\t", "\n", -1 );
}

input("X00001", "X00001");
input("X00002", "X00002");
input("X00003", "X00003");
input("X00004", "X00004");
input("X00005", "X00005");
input("X00006", "X00006");
input("X00007", "X00007");
input("X00008", "X00008");
input("X00009", "X00009");
input("X00010", "X00010");
input("X00011", "X00011");
input("X00012", "X00012");
input("X00013", "X00013");
input("X00014", "X00014");
input("X00015", "X00015");
input("X00016", "X00016");
input("X00017", "X00017");
input("X00018", "X00018");
input("X00019", "X00019");
input("X00020", "X00020");
input("X00021", "X00021");
input("X00022", "X00022");
input("X00023", "X00023");
input("X00024", "X00024");
input("X00025", "X00025");
input("X00026", "X00026");
input("X00027", "X00027");
input("X00028", "X00028");
input("X00029", "X00029");
input("X00030", "X00030");
input("X00031", "X00031");
input("X00032", "X00032");
input("X00033", "X00033");
input("X00034", "X00034");
input("X00035", "X00035");
input("X00036", "X00036");
input("X00037", "X00037");
input("X00038", "X00038");
input("X00039", "X00039");
input("X00040", "X00040");
input("X00041", "X00041");
input("X00042", "X00042");
input("X00043", "X00043");
input("X00044", "X00044");
input("X00045", "X00045");
input("X00046", "X00046");
input("X00047", "X00047");
input("X00048", "X00048");
input("X00049", "X00049");
input("X00050", "X00050");
input("X00051", "X00051");
input("X00052", "X00052");
input("X00053", "X00053");
input("X00054", "X00054");
input("X00055", "X00055");
input("X00056", "X00056");
input("X00057", "X00057");
input("X00058", "X00058");
input("X00059", "X00059");
input("X00060", "X00060");
input("X00061", "X00061");
input("X00062", "X00062");
input("X00063", "X00063");
input("X00064", "X00064");
input("X00065", "X00065");
input("X00066", "X00066");
input("X00067", "X00067");
input("X00068", "X00068");
input("X00069", "X00069");
input("X00070", "X00070");
input("X00071", "X00071");
input("X00072", "X00072");
input("X00073", "X00073");
input("X00074", "X00074");
input("X00075", "X00075");
input("X00076", "X00076");
input("X00077", "X00077");
input("X00078", "X00078");
input("X00079", "X00079");
input("X00080", "X00080");
input("X00081", "X00081");
input("X00082", "X00082");
input("X00083", "X00083");
input("X00084", "X00084");
input("X00085", "X00085");
input("X00086", "X00086");
input("X00087", "X00087");
input("X00088", "X00088");
input("X00089", "X00089");
input("X00090", "X00090");
input("X00091", "X00091");
input("X00092", "X00092");
input("X00093", "X00093");
input("X00094", "X00094");
input("X00095", "X00095");
input("X00096", "X00096");
input("X00097", "X00097");
input("X00098", "X00098");
input("X00099", "X00099");
input("X00100", "X00100");
input("X00101", "X00101");
input("X00102", "X00102");
input("X00103", "X00103");
input("X00104", "X00104");
input("X00105", "X00105");
input("X00106", "X00106");
input("X00107", "X00107");
input("X00108", "X00108");
input("X00109", "X00109");
input("X00110", "X00110");
input("X00111", "X00111");
input("X00112", "X00112");
input("X00113", "X00113");
input("X00114", "X00114");
input("X00115", "X00115");
input("X00116", "X00116");
input("X00117", "X00117");
input("X00118", "X00118");
input("X00119", "X00119");
input("X00120", "X00120");
input("X00121", "X00121");
input("X00122", "X00122");
input("X00123", "X00123");
input("X00124", "X00124");
input("X00125", "X00125");
input("X00126", "X00126");
input("X00127", "X00127");
input("X00128", "X00128");
input("X00129", "X00129");
input("X00130", "X00130");
input("X00131", "X00131");
input("X00132", "X00132");
input("X00133", "X00133");
input("X00134", "X00134");
input("X00135", "X00135");
input("X00136", "X00136");
input("X00137", "X00137");
input("X00138", "X00138");
input("X00139", "X00139");
input("X00140", "X00140");
input("X00141", "X00141");
input("X00142", "X00142");
input("X00143", "X00143");
input("X00144", "X00144");
input("X00145", "X00145");
input("X00146", "X00146");
input("X00147", "X00147");
input("X00148", "X00148");
input("X00149", "X00149");
input("X00150", "X00150");
input("X00151", "X00151");
input("X00152", "X00152");
input("X00153", "X00153");
input("X00154", "X00154");
input("X00155", "X00155");
input("X00156", "X00156");
input("X00157", "X00157");
input("X00158", "X00158");
input("X00159", "X00159");
input("X00160", "X00160");
input("X00161", "X00161");
input("X00162", "X00162");
input("X00163", "X00163");
input("X00164", "X00164");
input("X00165", "X00165");
input("X00166", "X00166");
input("X00167", "X00167");
input("X00168", "X00168");
input("X00169", "X00169");
input("X00170", "X00170");
input("X00171", "X00171");
input("X00172", "X00172");
input("X00173", "X00173");
input("X00174", "X00174");
input("X00175", "X00175");
input("X00176", "X00176");
input("X00177", "X00177");
input("X00178", "X00178");
input("X00179", "X00179");
input("X00180", "X00180");
input("X00181", "X00181");
input("X00182", "X00182");
input("X00183", "X00183");
input("X00184", "X00184");
input("X00185", "X00185");
input("X00186", "X00186");
input("X00187", "X00187");
input("X00188", "X00188");
input("X00189", "X00189");
input("X00190", "X00190");
input("X00191", "X00191");
input("X00192", "X00192");
input("X00193", "X00193");
input("X00194", "X00194");
input("X00195", "X00195");
input("X00196", "X00196");
input("X00197", "X00197");
input("X00198", "X00198");
input("X00199", "X00199");
input("X00200", "X00200");
input("X00201", "X00201");
input("X00202", "X00202");
input("X00203", "X00203");
input("X00204", "X00204");
input("X00205", "X00205");
input("X00206", "X00206");
input("X00207", "X00207");
input("X00208", "X00208");
input("X00209", "X00209");
input("X00210", "X00210");
input("X00211", "X00211");
input("X00212", "X00212");
input("X00213", "X00213");
input("X00214", "X00214");
input("X00215", "X00215");
input("X00216", "X00216");
input("X00217", "X00217");
input("X00218", "X00218");
input("X00219", "X00219");
input("X00220", "X00220");
input("X00221", "X00221");
input("X00222", "X00222");
input("X00223", "X00223");
input("X00224", "X00224");
input("X00225", "X00225");
input("X00226", "X00226");
input("X00227", "X00227");
input("X00228", "X00228");
input("X00229", "X00229");
input("X00230", "X00230");
input("X00231", "X00231");
input("X00232", "X00232");
input("X00233", "X00233");
input("X00234", "X00234");
input("X00235", "X00235");
input("X00236", "X00236");
input("X00237", "X00237");
input("X00238", "X00238");
input("X00239", "X00239");
input("X00240", "X00240");
input("X00241", "X00241");
input("X00242", "X00242");
input("X00243", "X00243");
input("X00244", "X00244");
input("X00245", "X00245");
input("X00246", "X00246");
input("X00247", "X00247");
input("X00248", "X00248");
input("X00249", "X00249");
input("X00250", "X00250");
input("X00251", "X00251");
input("X00252", "X00252");
input("X00253", "X00253");
input("X00254", "X00254");
input("X00255", "X00255");
input("X00256", "X00256");
input("X00257", "X00257");
input("X00258", "X00258");
input("X00259", "X00259");
input("X00260", "X00260");
input("X00261", "X00261");
input("X00262", "X00262");
input("X00263", "X00263");
input("X00264", "X00264");
input("X00265", "X00265");
input("X00266", "X00266");
input("X00267", "X00267");
input("X00268", "X00268");
input("X00269", "X00269");
input("X00270", "X00270");
input("X00271", "X00271");
input("X00272", "X00272");
input("X00273", "X00273");
input("X00274", "X00274");
input("X00275", "X00275");
input("X00276", "X00276");
input("X00277", "X00277");
input("X00278", "X00278");
input("X00279", "X00279");
input("X00280", "X00280");
input("X00281", "X00281");
input("X00282", "X00282");
input("X00283", "X00283");
input("X00284", "X00284");
input("X00285", "X00285");
input("X00286", "X00286");
input("X00287", "X00287");
input("X00288", "X00288");
input("X00289", "X00289");
input("X00290", "X00290");
input("X00291", "X00291");
input("X00292", "X00292");
input("X00293", "X00293");
input("X00294", "X00294");
input("X00295", "X00295");
input("X00296", "X00296");
input("X00297", "X00297");
input("X00298", "X00298");
input("X00299", "X00299");
input("X00300", "X00300");
input("X00301", "X00301");
input("X00302", "X00302");
input("X00303", "X00303");
input("X00304", "X00304");
input("X00305", "X00305");
input("X00306", "X00306");
input("X00307", "X00307");
input("X00308", "X00308");
input("X00309", "X00309");
input("X00310", "X00310");
input("X00311", "X00311");
input("X00312", "X00312");
input("X00313", "X00313");
input("X00314", "X00314");
input("X00315", "X00315");
input("X00316", "X00316");
input("X00317", "X00317");
input("X00318", "X00318");
input("X00319", "X00319");
input("X00320", "X00320");
input("X00321", "X00321");
input("X00322", "X00322");
input("X00323", "X00323");
input("X00324", "X00324");
input("X00325", "X00325");
input("X00326", "X00326");
input("X00327", "X00327");
input("X00328", "X00328");
input("X00329", "X00329");
input("X00330", "X00330");
input("X00331", "X00331");
input("X00332", "X00332");
input("X00333", "X00333");
input("X00334", "X00334");
input("X00335", "X00335");
input("X00336", "X00336");
input("X00337", "X00337");
input("X00338", "X00338");
input("X00339", "X00339");
input("X00340", "X00340");
input("X00341", "X00341");
input("X00342", "X00342");
input("X00343", "X00343");
input("X00344", "X00344");
input("X00345", "X00345");
input("X00346", "X00346");
input("X00347", "X00347");
input("X00348", "X00348");
input("X00349", "X00349");
input("X00350", "X00350");
input("X00351", "X00351");
input("X00352", "X00352");
input("X00353", "X00353");
input("X00354", "X00354");
input("X00355", "X00355");
input("X00356", "X00356");
input("X00357", "X00357");
input("X00358", "X00358");
input("X00359", "X00359");
input("X00360", "X00360");
input("X00361", "X00361");
input("X00362", "X00362");
input("X00363", "X00363");
input("X00364", "X00364");
input("X00365", "X00365");
input("X00366", "X00366");
input("X00367", "X00367");
input("X00368", "X00368");
input("X00369", "X00369");
input("X00370", "X00370");
input("X00371", "X00371");
input("X00372", "X00372");
input("X00373", "X00373");
input("X00374", "X00374");
input("X00375", "X00375");
input("X00376", "X00376");
input("X00377", "X00377");
input("X00378", "X00378");
input("X00379", "X00379");
input("X00380", "X00380");
input("X00381", "X00381");
input("X00382", "X00382");
input("X00383", "X00383");
input("X00384", "X00384");
input("X00385", "X00385");
input("X00386", "X00386");
input("X00387", "X00387");
input("X00388", "X00388");
input("X00389", "X00389");
input("X00390", "X00390");
input("X00391", "X00391");
input("X00392", "X00392");
input("X00393", "X00393");
input("X00394", "X00394");
input("X00395", "X00395");
input("X00396", "X00396");
input("X00397", "X00397");
input("X00398", "X00398");
input("X00399", "X00399");
input("X00400", "X00400");
input("X00401", "X00401");
input("X00402", "X00402");
input("X00403", "X00403");
input("X00404", "X00404");
input("X00405", "X00405");
input("X00406", "X00406");
input("X00407", "X00407");
input("X00408", "X00408");
input("X00409", "X00409");
input("X00410", "X00410");
input("X00411", "X00411");
input("X00412", "X00412");
input("X00413", "X00413");
input("X00414", "X00414");
input("X00415", "X00415");
input("X00416", "X00416");
input("X00417", "X00417");
input("X00418", "X00418");
input("X00419", "X00419");
input("X00420", "X00420");
input("X00421", "X00421");
input("X00422", "X00422");
input("X00423", "X00423");
input("X00424", "X00424");
input("X00425", "X00425");
input("X00426", "X00426");
input("X00427", "X00427");
input("X00428", "X00428");
input("X00429", "X00429");
input("X00430", "X00430");
input("X00431", "X00431");
input("X00432", "X00432");
input("X00433", "X00433");
input("X00434", "X00434");
input("X00435", "X00435");
input("X00436", "X00436");
input("X00437", "X00437");
input("X00438", "X00438");
input("X00439", "X00439");
input("X00440", "X00440");
input("X00441", "X00441");
input("X00442", "X00442");
input("X00443", "X00443");
input("X00444", "X00444");
input("X00445", "X00445");
input("X00446", "X00446");
input("X00447", "X00447");
input("X00448", "X00448");
input("X00449", "X00449");
input("X00450", "X00450");
input("X00451", "X00451");
input("X00452", "X00452");
input("X00453", "X00453");
input("X00454", "X00454");
input("X00455", "X00455");
input("X00456", "X00456");
input("X00457", "X00457");
input("X00458", "X00458");
input("X00459", "X00459");
input("X00460", "X00460");
input("X00461", "X00461");
input("X00462", "X00462");
input("X00463", "X00463");
input("X00464", "X00464");
input("X00465", "X00465");
input("X00466", "X00466");
input("X00467", "X00467");
input("X00468", "X00468");
input("X00469", "X00469");
input("X00470", "X00470");
input("X00471", "X00471");
input("X00472", "X00472");
input("X00473", "X00473");
input("X00474", "X00474");
input("X00475", "X00475");
input("X00476", "X00476");
input("X00477", "X00477");
input("X00478", "X00478");
input("X00479", "X00479");
input("X00480", "X00480");
input("X00481", "X00481");
input("X00482", "X00482");
input("X00483", "X00483");
input("X00484", "X00484");
input("X00485", "X00485");
input("X00486", "X00486");
input("X00487", "X00487");
input("X00488", "X00488");
input("X00489", "X00489");
input("X00490", "X00490");
input("X00491", "X00491");
input("X00492", "X00492");
input("X00493", "X00493");
input("X00494", "X00494");
input("X00495", "X00495");
input("X00496", "X00496");
input("X00497", "X00497");
input("X00498", "X00498");
input("X00499", "X00499");
input("X00500", "X00500");
input("X00501", "X00501");
input("X00502", "X00502");
input("X00503", "X00503");
input("X00504", "X00504");
input("X00505", "X00505");
input("X00506", "X00506");
input("X00507", "X00507");
input("X00508", "X00508");
input("X00509", "X00509");
input("X00510", "X00510");
input("X00511", "X00511");
input("X00512", "X00512");
input("X00513", "X00513");
input("X00514", "X00514");
input("X00515", "X00515");
input("X00516", "X00516");
input("X00517", "X00517");
input("X00518", "X00518");
input("X00519", "X00519");
input("X00520", "X00520");
input("X00521", "X00521");
input("X00522", "X00522");
input("X00523", "X00523");
input("X00524", "X00524");
input("X00525", "X00525");
input("X00526", "X00526");
input("X00527", "X00527");
input("X00528", "X00528");
input("X00529", "X00529");
input("X00530", "X00530");
input("X00531", "X00531");
input("X00532", "X00532");
input("X00533", "X00533");
input("X00534", "X00534");
input("X00535", "X00535");
input("X00536", "X00536");
input("X00537", "X00537");
input("X00538", "X00538");
input("X00539", "X00539");
input("X00540", "X00540");
input("X00541", "X00541");
input("X00542", "X00542");
input("X00543", "X00543");
input("X00544", "X00544");
input("X00545", "X00545");
input("X00546", "X00546");
input("X00547", "X00547");
input("X00548", "X00548");
input("X00549", "X00549");
input("X00550", "X00550");
input("X00551", "X00551");
input("X00552", "X00552");
input("X00553", "X00553");
input("X00554", "X00554");
input("X00555", "X00555");
input("X00556", "X00556");
input("X00557", "X00557");
input("X00558", "X00558");
input("X00559", "X00559");
input("X00560", "X00560");
input("X00561", "X00561");
input("X00562", "X00562");
input("X00563", "X00563");
input("X00564", "X00564");
input("X00565", "X00565");
input("X00566", "X00566");
input("X00567", "X00567");
input("X00568", "X00568");
input("X00569", "X00569");
input("X00570", "X00570");
input("X00571", "X00571");
input("X00572", "X00572");
input("X00573", "X00573");
input("X00574", "X00574");
input("X00575", "X00575");
input("X00576", "X00576");
input("X00577", "X00577");
input("X00578", "X00578");
input("X00579", "X00579");
input("X00580", "X00580");
input("X00581", "X00581");
input("X00582", "X00582");
input("X00583", "X00583");
input("X00584", "X00584");
input("X00585", "X00585");
input("X00586", "X00586");
input("X00587", "X00587");
input("X00588", "X00588");
input("X00589", "X00589");
input("X00590", "X00590");
input("X00591", "X00591");
input("X00592", "X00592");
input("X00593", "X00593");
input("X00594", "X00594");
input("X00595", "X00595");
input("X00596", "X00596");
input("X00597", "X00597");
input("X00598", "X00598");
input("X00599", "X00599");
input("X00600", "X00600");
input("X00601", "X00601");
input("X00602", "X00602");
input("X00603", "X00603");
input("X00604", "X00604");
input("X00605", "X00605");
input("X00606", "X00606");
input("X00607", "X00607");
input("X00608", "X00608");
input("X00609", "X00609");
input("X00610", "X00610");
input("X00611", "X00611");
input("X00612", "X00612");
input("X00613", "X00613");
input("X00614", "X00614");
input("X00615", "X00615");
input("X00616", "X00616");
input("X00617", "X00617");
input("X00618", "X00618");
input("X00619", "X00619");
input("X00620", "X00620");
input("X00621", "X00621");
input("X00622", "X00622");
input("X00623", "X00623");
input("X00624", "X00624");
input("X00625", "X00625");
input("X00626", "X00626");
input("X00627", "X00627");
input("X00628", "X00628");
input("X00629", "X00629");
input("X00630", "X00630");
input("X00631", "X00631");
input("X00632", "X00632");
input("X00633", "X00633");
input("X00634", "X00634");
input("X00635", "X00635");
input("X00636", "X00636");
input("X00637", "X00637");
input("X00638", "X00638");
input("X00639", "X00639");
input("X00640", "X00640");
input("X00641", "X00641");
input("X00642", "X00642");
input("X00643", "X00643");
input("X00644", "X00644");
input("X00645", "X00645");
input("X00646", "X00646");
input("X00647", "X00647");
input("X00648", "X00648");
input("X00649", "X00649");
input("X00650", "X00650");
input("X00651", "X00651");
input("X00652", "X00652");
input("X00653", "X00653");
input("X00654", "X00654");
input("X00655", "X00655");
input("X00656", "X00656");
input("X00657", "X00657");
input("X00658", "X00658");
input("X00659", "X00659");
input("X00660", "X00660");
input("X00661", "X00661");
input("X00662", "X00662");
input("X00663", "X00663");
input("X00664", "X00664");
input("X00665", "X00665");
input("X00666", "X00666");
input("X00667", "X00667");
input("X00668", "X00668");
input("X00669", "X00669");
input("X00670", "X00670");
input("X00671", "X00671");
input("X00672", "X00672");
input("X00673", "X00673");
input("X00674", "X00674");
input("X00675", "X00675");
input("X00676", "X00676");
input("X00677", "X00677");
input("X00678", "X00678");
input("X00679", "X00679");
input("X00680", "X00680");
input("X00681", "X00681");
input("X00682", "X00682");
input("X00683", "X00683");
input("X00684", "X00684");
input("X00685", "X00685");
input("X00686", "X00686");
input("X00687", "X00687");
input("X00688", "X00688");
input("X00689", "X00689");
input("X00690", "X00690");
input("X00691", "X00691");
input("X00692", "X00692");
input("X00693", "X00693");
input("X00694", "X00694");
input("X00695", "X00695");
input("X00696", "X00696");
input("X00697", "X00697");
input("X00698", "X00698");
input("X00699", "X00699");
input("X00700", "X00700");
input("X00701", "X00701");
input("X00702", "X00702");
input("X00703", "X00703");
input("X00704", "X00704");
input("X00705", "X00705");
input("X00706", "X00706");
input("X00707", "X00707");
input("X00708", "X00708");
input("X00709", "X00709");
input("X00710", "X00710");
input("X00711", "X00711");
input("X00712", "X00712");
input("X00713", "X00713");
input("X00714", "X00714");
input("X00715", "X00715");
input("X00716", "X00716");
input("X00717", "X00717");
input("X00718", "X00718");
input("X00719", "X00719");
input("X00720", "X00720");
input("X00721", "X00721");
input("X00722", "X00722");
input("X00723", "X00723");
input("X00724", "X00724");
input("X00725", "X00725");
input("X00726", "X00726");
input("X00727", "X00727");
input("X00728", "X00728");
input("X00729", "X00729");
input("X00730", "X00730");
input("X00731", "X00731");
input("X00732", "X00732");
input("X00733", "X00733");
input("X00734", "X00734");
input("X00735", "X00735");
input("X00736", "X00736");
input("X00737", "X00737");
input("X00738", "X00738");
input("X00739", "X00739");
input("X00740", "X00740");
input("X00741", "X00741");
input("X00742", "X00742");
input("X00743", "X00743");
input("X00744", "X00744");
input("X00745", "X00745");
input("X00746", "X00746");
input("X00747", "X00747");
input("X00748", "X00748");
input("X00749", "X00749");
input("X00750", "X00750");
input("X00751", "X00751");
input("X00752", "X00752");
input("X00753", "X00753");
input("X00754", "X00754");
input("X00755", "X00755");
input("X00756", "X00756");
input("X00757", "X00757");
input("X00758", "X00758");
input("X00759", "X00759");
input("X00760", "X00760");
input("X00761", "X00761");
input("X00762", "X00762");
input("X00763", "X00763");
input("X00764", "X00764");
input("X00765", "X00765");
input("X00766", "X00766");
input("X00767", "X00767");
input("X00768", "X00768");
input("X00769", "X00769");
input("X00770", "X00770");
input("X00771", "X00771");
input("X00772", "X00772");
input("X00773", "X00773");
input("X00774", "X00774");
input("X00775", "X00775");
input("X00776", "X00776");
input("X00777", "X00777");
input("X00778", "X00778");
input("X00779", "X00779");
input("X00780", "X00780");
input("X00781", "X00781");
input("X00782", "X00782");
input("X00783", "X00783");
input("X00784", "X00784");
input("X00785", "X00785");
input("X00786", "X00786");
input("X00787", "X00787");
input("X00788", "X00788");
input("X00789", "X00789");
input("X00790", "X00790");
input("X00791", "X00791");
input("X00792", "X00792");
input("X00793", "X00793");
input("X00794", "X00794");
input("X00795", "X00795");
input("X00796", "X00796");
input("X00797", "X00797");
input("X00798", "X00798");
input("X00799", "X00799");
input("X00800", "X00800");
input("X00801", "X00801");
input("X00802", "X00802");
input("X00803", "X00803");
input("X00804", "X00804");
input("X00805", "X00805");
input("X00806", "X00806");
input("X00807", "X00807");
input("X00808", "X00808");
input("X00809", "X00809");
input("X00810", "X00810");
input("X00811", "X00811");
input("X00812", "X00812");
input("X00813", "X00813");
input("X00814", "X00814");
input("X00815", "X00815");
input("X00816", "X00816");
input("X00817", "X00817");
input("X00818", "X00818");
input("X00819", "X00819");
input("X00820", "X00820");
input("X00821", "X00821");
input("X00822", "X00822");
input("X00823", "X00823");
input("X00824", "X00824");
input("X00825", "X00825");
input("X00826", "X00826");
input("X00827", "X00827");
input("X00828", "X00828");
input("X00829", "X00829");
input("X00830", "X00830");
input("X00831", "X00831");
input("X00832", "X00832");
input("X00833", "X00833");
input("X00834", "X00834");
input("X00835", "X00835");
input("X00836", "X00836");
input("X00837", "X00837");
input("X00838", "X00838");
input("X00839", "X00839");
input("X00840", "X00840");
input("X00841", "X00841");
input("X00842", "X00842");
input("X00843", "X00843");
input("X00844", "X00844");
input("X00845", "X00845");
input("X00846", "X00846");
input("X00847", "X00847");
input("X00848", "X00848");
input("X00849", "X00849");
input("X00850", "X00850");
input("X00851", "X00851");
input("X00852", "X00852");
input("X00853", "X00853");
input("X00854", "X00854");
input("X00855", "X00855");
input("X00856", "X00856");
input("X00857", "X00857");
input("X00858", "X00858");
input("X00859", "X00859");
input("X00860", "X00860");
input("X00861", "X00861");
input("X00862", "X00862");
input("X00863", "X00863");
input("X00864", "X00864");
input("X00865", "X00865");
input("X00866", "X00866");
input("X00867", "X00867");
input("X00868", "X00868");
input("X00869", "X00869");
input("X00870", "X00870");
input("X00871", "X00871");
input("X00872", "X00872");
input("X00873", "X00873");
input("X00874", "X00874");
input("X00875", "X00875");
input("X00876", "X00876");
input("X00877", "X00877");
input("X00878", "X00878");
input("X00879", "X00879");
input("X00880", "X00880");
input("X00881", "X00881");
input("X00882", "X00882");
input("X00883", "X00883");
input("X00884", "X00884");
input("X00885", "X00885");
input("X00886", "X00886");
input("X00887", "X00887");
input("X00888", "X00888");
input("X00889", "X00889");
input("X00890", "X00890");
input("X00891", "X00891");
input("X00892", "X00892");
input("X00893", "X00893");
input("X00894", "X00894");
input("X00895", "X00895");
input("X00896", "X00896");
input("X00897", "X00897");
input("X00898", "X00898");
input("X00899", "X00899");
input("X00900", "X00900");
input("X00901", "X00901");
input("X00902", "X00902");
input("X00903", "X00903");
input("X00904", "X00904");
input("X00905", "X00905");
input("X00906", "X00906");
input("X00907", "X00907");
input("X00908", "X00908");
input("X00909", "X00909");
input("X00910", "X00910");
input("X00911", "X00911");
input("X00912", "X00912");
input("X00913", "X00913");
input("X00914", "X00914");
input("X00915", "X00915");
input("X00916", "X00916");
input("X00917", "X00917");
input("X00918", "X00918");
input("X00919", "X00919");
input("X00920", "X00920");
input("X00921", "X00921");
input("X00922", "X00922");
input("X00923", "X00923");
input("X00924", "X00924");
input("X00925", "X00925");
input("X00926", "X00926");
input("X00927", "X00927");
input("X00928", "X00928");
input("X00929", "X00929");
input("X00930", "X00930");
input("X00931", "X00931");
input("X00932", "X00932");
input("X00933", "X00933");
input("X00934", "X00934");
input("X00935", "X00935");
input("X00936", "X00936");
input("X00937", "X00937");
input("X00938", "X00938");
input("X00939", "X00939");
input("X00940", "X00940");
input("X00941", "X00941");
input("X00942", "X00942");
input("X00943", "X00943");
input("X00944", "X00944");
input("X00945", "X00945");
input("X00946", "X00946");
input("X00947", "X00947");
input("X00948", "X00948");
input("X00949", "X00949");
input("X00950", "X00950");
input("X00951", "X00951");
input("X00952", "X00952");
input("X00953", "X00953");
input("X00954", "X00954");
input("X00955", "X00955");
input("X00956", "X00956");
input("X00957", "X00957");
input("X00958", "X00958");
input("X00959", "X00959");
input("X00960", "X00960");
input("X00961", "X00961");
input("X00962", "X00962");
input("X00963", "X00963");
input("X00964", "X00964");
input("X00965", "X00965");
input("X00966", "X00966");
input("X00967", "X00967");
input("X00968", "X00968");
input("X00969", "X00969");
input("X00970", "X00970");
input("X00971", "X00971");
input("X00972", "X00972");
input("X00973", "X00973");
input("X00974", "X00974");
input("X00975", "X00975");
input("X00976", "X00976");
input("X00977", "X00977");
input("X00978", "X00978");
input("X00979", "X00979");
input("X00980", "X00980");
input("X00981", "X00981");
input("X00982", "X00982");
input("X00983", "X00983");
input("X00984", "X00984");
input("X00985", "X00985");
input("X00986", "X00986");
input("X00987", "X00987");
input("X00988", "X00988");
input("X00989", "X00989");
input("X00990", "X00990");
input("X00991", "X00991");
input("X00992", "X00992");
input("X00993", "X00993");
input("X00994", "X00994");
input("X00995", "X00995");
input("X00996", "X00996");
input("X00997", "X00997");
input("X00998", "X00998");
input("X00999", "X00999");
input("X01000", "X01000");
input("X01001", "X01001");
input("X01002", "X01002");
input("X01003", "X01003");
input("X01004", "X01004");
input("X01005", "X01005");
input("X01006", "X01006");
input("X01007", "X01007");
input("X01008", "X01008");
input("X01009", "X01009");
input("X01010", "X01010");
input("X01011", "X01011");
input("X01012", "X01012");
input("X01013", "X01013");
input("X01014", "X01014");
input("X01015", "X01015");
input("X01016", "X01016");
input("X01017", "X01017");
input("X01018", "X01018");
input("X01019", "X01019");
input("X01020", "X01020");
input("X01021", "X01021");
input("X01022", "X01022");
input("X01023", "X01023");
input("X01024", "X01024");
input("X01025", "X01025");
input("X01026", "X01026");
input("X01027", "X01027");
input("X01028", "X01028");
input("X01029", "X01029");
input("X01030", "X01030");
input("X01031", "X01031");
input("X01032", "X01032");
input("X01033", "X01033");
input("X01034", "X01034");
input("X01035", "X01035");
input("X01036", "X01036");
input("X01037", "X01037");
input("X01038", "X01038");
input("X01039", "X01039");
input("X01040", "X01040");
input("X01041", "X01041");
input("X01042", "X01042");
input("X01043", "X01043");
input("X01044", "X01044");
input("X01045", "X01045");
input("X01046", "X01046");
input("X01047", "X01047");
input("X01048", "X01048");
input("X01049", "X01049");
input("X01050", "X01050");
input("X01051", "X01051");
input("X01052", "X01052");
input("X01053", "X01053");
input("X01054", "X01054");
input("X01055", "X01055");
input("X01056", "X01056");
input("X01057", "X01057");
input("X01058", "X01058");
input("X01059", "X01059");
input("X01060", "X01060");
input("X01061", "X01061");
input("X01062", "X01062");
input("X01063", "X01063");
input("X01064", "X01064");
input("X01065", "X01065");
input("X01066", "X01066");
input("X01067", "X01067");
input("X01068", "X01068");
input("X01069", "X01069");
input("X01070", "X01070");
input("X01071", "X01071");
input("X01072", "X01072");
input("X01073", "X01073");
input("X01074", "X01074");
input("X01075", "X01075");
input("X01076", "X01076");
input("X01077", "X01077");
input("X01078", "X01078");
input("X01079", "X01079");
input("X01080", "X01080");
input("X01081", "X01081");
input("X01082", "X01082");
input("X01083", "X01083");
input("X01084", "X01084");
input("X01085", "X01085");
input("X01086", "X01086");
input("X01087", "X01087");
input("X01088", "X01088");
input("X01089", "X01089");
input("X01090", "X01090");
input("X01091", "X01091");
input("X01092", "X01092");
input("X01093", "X01093");
input("X01094", "X01094");
input("X01095", "X01095");
input("X01096", "X01096");
input("X01097", "X01097");
input("X01098", "X01098");
input("X01099", "X01099");
input("X01100", "X01100");
input("X01101", "X01101");
input("X01102", "X01102");
input("X01103", "X01103");
input("X01104", "X01104");
input("X01105", "X01105");
input("X01106", "X01106");
input("X01107", "X01107");
input("X01108", "X01108");
input("X01109", "X01109");
input("X01110", "X01110");
input("X01111", "X01111");
input("X01112", "X01112");
input("X01113", "X01113");
input("X01114", "X01114");
input("X01115", "X01115");
input("X01116", "X01116");
input("X01117", "X01117");
input("X01118", "X01118");
input("X01119", "X01119");
input("X01120", "X01120");
input("X01121", "X01121");
input("X01122", "X01122");
input("X01123", "X01123");
input("X01124", "X01124");
input("X01125", "X01125");
input("X01126", "X01126");
input("X01127", "X01127");
input("X01128", "X01128");
input("X01129", "X01129");
input("X01130", "X01130");
input("X01131", "X01131");
input("X01132", "X01132");
input("X01133", "X01133");
input("X01134", "X01134");
input("X01135", "X01135");
input("X01136", "X01136");
input("X01137", "X01137");
input("X01138", "X01138");
input("X01139", "X01139");
input("X01140", "X01140");
input("X01141", "X01141");
input("X01142", "X01142");
input("X01143", "X01143");
input("X01144", "X01144");
input("X01145", "X01145");
input("X01146", "X01146");
input("X01147", "X01147");
input("X01148", "X01148");
input("X01149", "X01149");
input("X01150", "X01150");
input("X01151", "X01151");
input("X01152", "X01152");
input("X01153", "X01153");
input("X01154", "X01154");
input("X01155", "X01155");
input("X01156", "X01156");
input("X01157", "X01157");
input("X01158", "X01158");
input("X01159", "X01159");
input("X01160", "X01160");
input("X01161", "X01161");
input("X01162", "X01162");
input("X01163", "X01163");
input("X01164", "X01164");
input("X01165", "X01165");
input("X01166", "X01166");
input("X01167", "X01167");
input("X01168", "X01168");
input("X01169", "X01169");
input("X01170", "X01170");
input("X01171", "X01171");
input("X01172", "X01172");
input("X01173", "X01173");
input("X01174", "X01174");
input("X01175", "X01175");
input("X01176", "X01176");
input("X01177", "X01177");
input("X01178", "X01178");
input("X01179", "X01179");
input("X01180", "X01180");
input("X01181", "X01181");
input("X01182", "X01182");
input("X01183", "X01183");
input("X01184", "X01184");
input("X01185", "X01185");
input("X01186", "X01186");
input("X01187", "X01187");
input("X01188", "X01188");
input("X01189", "X01189");
input("X01190", "X01190");
input("X01191", "X01191");
input("X01192", "X01192");
input("X01193", "X01193");
input("X01194", "X01194");
input("X01195", "X01195");
input("X01196", "X01196");
input("X01197", "X01197");
input("X01198", "X01198");
input("X01199", "X01199");
input("X01200", "X01200");
input("X01201", "X01201");
input("X01202", "X01202");
input("X01203", "X01203");
input("X01204", "X01204");
input("X01205", "X01205");
input("X01206", "X01206");
input("X01207", "X01207");
input("X01208", "X01208");
input("X01209", "X01209");
input("X01210", "X01210");
input("X01211", "X01211");
input("X01212", "X01212");
input("X01213", "X01213");
input("X01214", "X01214");
input("X01215", "X01215");
input("X01216", "X01216");
input("X01217", "X01217");
input("X01218", "X01218");
input("X01219", "X01219");
input("X01220", "X01220");
input("X01221", "X01221");
input("X01222", "X01222");
input("X01223", "X01223");
input("X01224", "X01224");
input("X01225", "X01225");
input("X01226", "X01226");
input("X01227", "X01227");
input("X01228", "X01228");
input("X01229", "X01229");
input("X01230", "X01230");
input("X01231", "X01231");
input("X01232", "X01232");
input("X01233", "X01233");
input("X01234", "X01234");
input("X01235", "X01235");
input("X01236", "X01236");
input("X01237", "X01237");
input("X01238", "X01238");
input("X01239", "X01239");
input("X01240", "X01240");
input("X01241", "X01241");
input("X01242", "X01242");
input("X01243", "X01243");
input("X01244", "X01244");
input("X01245", "X01245");
input("X01246", "X01246");
input("X01247", "X01247");
input("X01248", "X01248");
input("X01249", "X01249");
input("X01250", "X01250");
input("X01251", "X01251");
input("X01252", "X01252");
input("X01253", "X01253");
input("X01254", "X01254");
input("X01255", "X01255");
input("X01256", "X01256");
input("X01257", "X01257");
input("X01258", "X01258");
input("X01259", "X01259");
input("X01260", "X01260");
input("X01261", "X01261");
input("X01262", "X01262");
input("X01263", "X01263");
input("X01264", "X01264");
input("X01265", "X01265");
input("X01266", "X01266");
input("X01267", "X01267");
input("X01268", "X01268");
input("X01269", "X01269");
input("X01270", "X01270");
input("X01271", "X01271");
input("X01272", "X01272");
input("X01273", "X01273");
input("X01274", "X01274");
input("X01275", "X01275");
input("X01276", "X01276");
input("X01277", "X01277");
input("X01278", "X01278");
input("X01279", "X01279");
input("X01280", "X01280");
input("X01281", "X01281");
input("X01282", "X01282");
input("X01283", "X01283");
input("X01284", "X01284");
input("X01285", "X01285");
input("X01286", "X01286");
input("X01287", "X01287");
input("X01288", "X01288");
input("X01289", "X01289");
input("X01290", "X01290");
input("X01291", "X01291");
input("X01292", "X01292");
input("X01293", "X01293");
input("X01294", "X01294");
input("X01295", "X01295");
input("X01296", "X01296");
input("X01297", "X01297");
input("X01298", "X01298");
input("X01299", "X01299");
input("X01300", "X01300");
input("X01301", "X01301");
input("X01302", "X01302");
input("X01303", "X01303");
input("X01304", "X01304");
input("X01305", "X01305");
input("X01306", "X01306");
input("X01307", "X01307");
input("X01308", "X01308");
input("X01309", "X01309");
input("X01310", "X01310");
input("X01311", "X01311");
input("X01312", "X01312");
input("X01313", "X01313");
input("X01314", "X01314");
input("X01315", "X01315");
input("X01316", "X01316");
input("X01317", "X01317");
input("X01318", "X01318");
input("X01319", "X01319");
input("X01320", "X01320");
input("X01321", "X01321");
input("X01322", "X01322");
input("X01323", "X01323");
input("X01324", "X01324");
input("X01325", "X01325");
input("X01326", "X01326");
input("X01327", "X01327");
input("X01328", "X01328");
input("X01329", "X01329");
input("X01330", "X01330");
input("X01331", "X01331");
input("X01332", "X01332");
input("X01333", "X01333");
input("X01334", "X01334");
input("X01335", "X01335");
input("X01336", "X01336");
input("X01337", "X01337");
input("X01338", "X01338");
input("X01339", "X01339");
input("X01340", "X01340");
input("X01341", "X01341");
input("X01342", "X01342");
input("X01343", "X01343");
input("X01344", "X01344");
input("X01345", "X01345");
input("X01346", "X01346");
input("X01347", "X01347");
input("X01348", "X01348");
input("X01349", "X01349");
input("X01350", "X01350");
input("X01351", "X01351");
input("X01352", "X01352");
input("X01353", "X01353");
input("X01354", "X01354");
input("X01355", "X01355");
input("X01356", "X01356");
input("X01357", "X01357");
input("X01358", "X01358");
input("X01359", "X01359");
input("X01360", "X01360");
input("X01361", "X01361");
input("X01362", "X01362");
input("X01363", "X01363");
input("X01364", "X01364");
input("X01365", "X01365");
input("X01366", "X01366");
input("X01367", "X01367");
commit();
EOF

