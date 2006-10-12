START TRANSACTION;

CREATE TABLE RC3(
	name varchar(20) NOT NULL,
	aliases varchar(20) NULL,
	PGC_name varchar(10) NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	good_position int NOT NULL,
	galactic_l float NULL,
	galactic_b float NULL,
	superGalactic_l float NULL,
	superGalactic_b float NULL,
	PosAngle float NULL,
	rc2_type char(7) NULL,
	rc2_typeSource char(6) NULL,
	T_type float NULL,
	T_typeErr float NULL,
	LC_rc2 float NULL,
	LC_rc2Err float NULL,
	n_L float NULL,
	B_T float NULL,
	B_TErr float NULL,
	B_Tsource char(1) NULL,
	B_T0 float NULL,
	M_B float NULL,
	M_BErr float NULL,
	M_FIR float NULL,
	M_21cm float NULL,
	M_21cmErr float NULL,
	BV_T float NULL,
	BV_TErr float NULL,
	BV_T0 float NULL,
	BV_e float NULL,
	BV_eErr float NULL,
	UB_T float NULL,
	UB_TErr float NULL,
	UB_T0 float NULL,
	UB_e float NULL,
	UB_eErr float NULL,
	lgD_25 float NULL,
	lgD_25Err float NULL,
	lgD_0 float NULL,
	lgR_25 float NULL,
	lgR_25Err float NULL,
	logA_e float NULL,
	logA_eErr float NULL,
	m_e float NULL,
	m_eErr float NULL,
	m_25 float NULL,
	m_25Err float NULL,
	A_B float NULL,
	A_HI_self float NULL,
	A_B_int float NULL,
	W_20 float NULL,
	W_20Err float NULL,
	W_50 float NULL,
	W_50Err float NULL,
	HI float NULL,
	V_21cm float NULL,
	V_21cmErr float NULL,
	V_opt float NULL,
	V_optErr float NULL,
	V_GSR float NULL,
	V_3K float NULL,
	htmID bigint NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	objId bigint NOT NULL,
 CONSTRAINT pk_RC3_objID PRIMARY KEY  
(
	objId
)
);
CREATE TABLE Stetson(
	name varchar(20) NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	B float NULL,
	BErr float NULL,
	nB_tot int NULL,
	nB int NULL,
	V float NULL,
	VErr float NULL,
	nV_tot int NULL,
	nV int NULL,
	R float NULL,
	RErr float NULL,
	nR_tot int NULL,
	nR int NULL,
	I float NULL,
	IErr float NULL,
	nI_tot int NULL,
	nI int NULL,
	vary float NULL,
	objid bigint NULL,
 CONSTRAINT pk_Stetson_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE QsoCatalogAll(
	SpecObjID bigint NOT NULL,
	bestObjID bigint NOT NULL,
	targetObjID bigint NOT NULL,
	zone int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	Confidence float NOT NULL,
	SpecObjQso tinyint NOT NULL,
	BestObjQso tinyint NOT NULL,
	TargetObjQso tinyint NOT NULL,
 CONSTRAINT pk_QsoCatalogAll_zone_ra_SpecObj PRIMARY KEY  
(
	zone,
	ra,
	SpecObjID,
	bestObjID,
	targetObjID
)
);
CREATE TABLE QsoConcordance(
	RowC real NOT NULL,
	ColC real NOT NULL,
	RowC_i real NOT NULL,
	ColC_i real NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	bestObjID bigint NOT NULL,
	targetObjID bigint NOT NULL,
	SpecObjID bigint NOT NULL,
	zone int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	QsoConfidence float NOT NULL,
	SpecObjFlagedQso tinyint NOT NULL,
	BestObjFlagedQso tinyint NOT NULL,
	TargetObjFlagedQso tinyint NOT NULL,
	sZ real NOT NULL,
	sZerr real NOT NULL,
	szConf real NOT NULL,
	szWarning int NOT NULL,
	sZStatus int NOT NULL,
	sSpecClass int NOT NULL,
	splate int NOT NULL,
	SFiberID int NOT NULL,
	sMjd int NOT NULL,
	sPrimTarget int NOT NULL,
	bType int NOT NULL,
	tType int NOT NULL,
	bFlags bigint NOT NULL,
	tFlags bigint NOT NULL,
	bPrimTarget int NOT NULL,
	tPrimTarget int NOT NULL,
	bPsfMag_u real NOT NULL,
	bPsfMag_g real NOT NULL,
	bPsfMag_r real NOT NULL,
	bPsfMag_i real NOT NULL,
	bPsfMag_z real NOT NULL,
	bPsfMagErr_u real NOT NULL,
	bPsfMagErr_g real NOT NULL,
	bPsfMagErr_r real NOT NULL,
	bPsfMagErr_i real NOT NULL,
	bPsfMagErr_z real NOT NULL,
	tPsfMag_u real NOT NULL,
	tPsfMag_g real NOT NULL,
	tPsfMag_r real NOT NULL,
	tPsfMag_i real NOT NULL,
	tPsfMag_z real NOT NULL,
	tPsfMagErr_u real NOT NULL,
	tPsfMagErr_g real NOT NULL,
	tPsfMagErr_r real NOT NULL,
	tPsfMagErr_i real NOT NULL,
	tPsfMagErr_z real NOT NULL,
 CONSTRAINT pk_QsoConcordance_zone_ra_dec_sp PRIMARY KEY  
(
	zone,
	ra,
	"dec",
	SpecObjID,
	bestObjID,
	targetObjID
)
);
CREATE TABLE Target(
	targetID bigint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field int NOT NULL,
	obj int NOT NULL,
	regionID int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	duplicate tinyint NOT NULL,
	htmID bigint NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	bestObjID bigint NOT NULL,
	specObjID bigint NOT NULL,
	bestMode tinyint NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_Target_targetID PRIMARY KEY  
(
	targetID
)
);
CREATE TABLE TilingRun(
	tileRun smallint NOT NULL,
	tileVersion varchar(32) NOT NULL,
	tilepId varchar(32) NOT NULL,
	programType int NOT NULL,
	programName varchar(32) NOT NULL,
	primTargetMask int NOT NULL,
	secTargetMask int NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TilingRun_tileRun PRIMARY KEY  
(
	tileRun
)
);
CREATE TABLE TargetParam(
	targetVersion varchar(32) NOT NULL,
	name varchar(32) NOT NULL,
	value varchar(32) NOT NULL,
 CONSTRAINT pk_TargetParam_targetVersion_nam PRIMARY KEY  
(
	targetVersion,
	name
)
);
CREATE TABLE Rmatrix(
	mode varchar(16) NOT NULL,
	"row" smallint NOT NULL,
	x float NOT NULL,
	y float NOT NULL,
	z float NOT NULL,
 CONSTRAINT pk_RMatrix_mode_row PRIMARY KEY  
(
	mode,
	"row"
)
);
CREATE TABLE Region(
	regionid bigint GENERATED ALWAYS AS IDENTITY NOT NULL,
	id bigint NOT NULL,
	type varchar(16) NOT NULL,
	comment varchar(1024) NOT NULL,
	ismask tinyint NOT NULL DEFAULT (0),
	area float NOT NULL DEFAULT (0),
	regionString text NOT NULL DEFAULT (''),
	sql text NOT NULL DEFAULT (''),
	xml text NOT NULL DEFAULT (''),
 CONSTRAINT pk_Region_regionId PRIMARY KEY  
(
	regionid
)
);
CREATE TABLE SiteDBs(
	dbname varchar(64) NOT NULL,
	description varchar(128) NOT NULL,
	active char(1) NOT NULL
);
CREATE TABLE QueryResults(
	query varchar(10) NOT NULL,
	cpu_sec float NOT NULL,
	elapsed_time float NOT NULL,
	physical_IO float NOT NULL,
	row_count bigint NOT NULL,
	time timestamp NOT NULL DEFAULT (localtimestamp),
	comment varchar(100) NULL DEFAULT (''),
 CONSTRAINT pk_QueryResults_query_time PRIMARY KEY  
(
	query,
	time
)
);
CREATE TABLE RecentQueries(
	ipAddr varchar(30) NOT NULL,
	lastQueryTime timestamp NOT NULL,
 CONSTRAINT pk_RecentQueries_ipAddr_lastQuer PRIMARY KEY  
(
	ipAddr,
	lastQueryTime
)
);
CREATE TABLE SiteConstants(
	name varchar(64) NOT NULL,
	value varchar(64) NOT NULL,
	comment varchar(256) NOT NULL,
 CONSTRAINT pk_SiteConstants_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE Glossary(
	"key" varchar(128) NOT NULL,
	name varchar(128) NOT NULL,
	ucd varchar(128) NOT NULL,
	"text" varchar(7200) NOT NULL,
 CONSTRAINT pk_Glossary_key PRIMARY KEY  
(
	"key"
)
);
CREATE TABLE Algorithm(
	"key" varchar(128) NOT NULL,
	name varchar(128) NOT NULL,
	ucd varchar(128) NOT NULL,
	"text" text NOT NULL,
 CONSTRAINT pk_Algorithm_key PRIMARY KEY  
(
	"key"
)
);
CREATE TABLE TableDesc(
	"key" varchar(128) NOT NULL,
	type varchar(128) NOT NULL,
	ucd varchar(128) NOT NULL,
	"text" varchar(7200) NOT NULL,
 CONSTRAINT pk_TableDesc_key PRIMARY KEY  
(
	"key"
)
);
CREATE TABLE DBObjects(
	name varchar(128) NOT NULL,
	type varchar(64) NOT NULL,
	access varchar(8) NOT NULL,
	description varchar(256) NOT NULL,
	"text" varchar(7200) NOT NULL,
	rank int NOT NULL DEFAULT (0),
 CONSTRAINT pk_DBObjects_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE History(
	id int GENERATED ALWAYS AS IDENTITY NOT NULL,
	filename varchar(128) NOT NULL,
	date varchar(64) NOT NULL,
	name varchar(64) NOT NULL,
	description varchar(7200) NOT NULL,
 CONSTRAINT pk_History_id PRIMARY KEY  
(
	id
)
);
CREATE TABLE Dependency(
	filename varchar(128) NOT NULL,
	parent varchar(128) NOT NULL,
	child varchar(128) NOT NULL,
 CONSTRAINT pk_Dependency_parent_child PRIMARY KEY  
(
	parent,
	child
)
);
CREATE TABLE PubHistory(
	name varchar(64) NOT NULL,
	nrows int NOT NULL,
	tend timestamp NOT NULL,
	loadversion int NOT NULL,
 CONSTRAINT pk_PubHistory_name_loadversion PRIMARY KEY  
(
	name,
	loadversion
)
);
CREATE TABLE LoadHistory(
	loadversion int NOT NULL,
	tstart timestamp NOT NULL,
	tend timestamp NOT NULL,
	dbname varchar(64) NOT NULL,
 CONSTRAINT pk_LoadHistory_loadVersion_tStar PRIMARY KEY  
(
	loadversion,
	tstart
)
);
CREATE TABLE "Diagnostics"(
	name varchar(64) NOT NULL,
	type varchar(8) NOT NULL,
	"count" bigint NULL,
 CONSTRAINT pk_Diagnostics_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE SiteDiagnostics(
	name varchar(64) NOT NULL,
	type varchar(8) NOT NULL,
	count bigint NULL,
 CONSTRAINT pk_SiteDiagnostics_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE Versions(
	event varchar(256) NOT NULL,
	previous varchar(64) NOT NULL,
	checksum varchar(64) NOT NULL,
	version varchar(64) NOT NULL,
	description varchar(256) NOT NULL,
	"text" varchar(2000) NOT NULL,
	who varchar(256) NOT NULL,
	"when" varchar(256) NOT NULL,
 CONSTRAINT pk_Versions_version PRIMARY KEY  
(
	version
)
);
CREATE TABLE PartitionMap(
	fileGroupName varchar(100) NOT NULL,
	size real NOT NULL,
	comment varchar(4000) NULL,
 CONSTRAINT pk_PartitionMap_fileGroupName PRIMARY KEY  
(
	fileGroupName
)
);
CREATE TABLE DataConstants(
	field varchar(128) NOT NULL,
	name varchar(128) NOT NULL,
	value bigint , --NOT NULL,
	description varchar(2000) NOT NULL,
 CONSTRAINT pk_DataConstants_field_name PRIMARY KEY  
(
	field,
	name
)
);
CREATE TABLE SDSSConstants(
	name varchar(32) NOT NULL,
	value float NOT NULL,
	unit varchar(32) NOT NULL,
	description varchar(2000) NOT NULL,
 CONSTRAINT pk_SDSSConstants_name PRIMARY KEY  
(
	name
)
);
CREATE TABLE StripeDefs(
	stripe int NOT NULL,
	hemisphere varchar(64) NOT NULL,
	eta float NOT NULL,
	lambdaMin float NOT NULL,
	lambdaMax float NOT NULL,
	htmArea varchar(1024) NOT NULL,
 CONSTRAINT pk_StripeDefs_stripe PRIMARY KEY  
(
	stripe
)
);
CREATE TABLE RunShift(
	run int NOT NULL,
	shift float NOT NULL,
 CONSTRAINT pk_RunShift_run PRIMARY KEY  
(
	run
)
);
CREATE TABLE ProfileDefs(
	bin int NOT NULL,
	cell int NOT NULL,
	sinc int NOT NULL,
	rInner float NOT NULL,
	rOuter float NOT NULL,
	aAnn float NOT NULL,
	aDisk float NOT NULL,
 CONSTRAINT pk_ProfileDefs_bin PRIMARY KEY  
(
	bin
)
);
CREATE TABLE Mask(
	maskID bigint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	mask smallint NOT NULL,
	filter tinyint NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	radius float NOT NULL,
	area varchar(4096) NOT NULL,
	type int NOT NULL,
	seeing real NOT NULL,
	cx float NOT NULL DEFAULT (0),
	cy float NOT NULL DEFAULT (0),
	cz float NOT NULL DEFAULT (0),
	htmID bigint NOT NULL DEFAULT (0),
 CONSTRAINT pk_Mask_maskID PRIMARY KEY  
(
	maskID
)
);
CREATE TABLE "Match"(
	objID1 bigint NOT NULL,
	objID2 bigint NOT NULL,
	run1 smallint NOT NULL,
	run2 smallint NOT NULL,
	type1 tinyint NOT NULL,
	type2 tinyint NOT NULL,
	mode1 tinyint NOT NULL,
	mode2 tinyint NOT NULL,
	distance float NOT NULL,
	miss char(1) NOT NULL,
	matchHead bigint NOT NULL,
 CONSTRAINT pk_Match_objID1_objID2 PRIMARY KEY  
(
	objID1,
	objID2
)
);
CREATE TABLE SpecObjAll(
	specObjID bigint NOT NULL,
	mjd int NOT NULL,
	plate smallint NOT NULL,
	tile smallint NOT NULL,
	xFocal float NOT NULL,
	yFocal float NOT NULL,
	fiberID smallint NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	zConf real NOT NULL,
	zStatus smallint NOT NULL,
	specClass smallint NOT NULL,
	zWarning int NOT NULL,
	velDisp real NOT NULL,
	velDispErr real NOT NULL,
	eCoeff_0 real NOT NULL,
	eCoeff_1 real NOT NULL,
	eCoeff_2 real NOT NULL,
	eCoeff_3 real NOT NULL,
	eCoeff_4 real NOT NULL,
	eClass real NOT NULL,
	nGood int NOT NULL,
	sn_0 real NOT NULL,
	sn_1 real NOT NULL,
	sn_2 real NOT NULL,
	mag_0 real NOT NULL,
	mag_1 real NOT NULL,
	mag_2 real NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	htmID bigint NOT NULL,
	fiberMag_u real NOT NULL,
	fiberMag_g real NOT NULL,
	fiberMag_r real NOT NULL,
	fiberMag_i real NOT NULL,
	fiberMag_z real NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	pixMin smallint NOT NULL,
	pixMax smallint NOT NULL,
	sci_sn real NOT NULL,
	smear_sn real NOT NULL,
	sm_leg_0 real NOT NULL,
	sm_leg_1 real NOT NULL,
	sm_leg_2 real NOT NULL,
	sciencePrimary smallint NOT NULL,
	loadVersion int NOT NULL,
	plateID bigint NOT NULL,
	spectrographID smallint NOT NULL,
	throughput int NOT NULL,
	objType int NOT NULL,
	objTypeName varchar(32) NOT NULL,
	targetID bigint NOT NULL,
	targetObjID bigint NOT NULL,
	bestObjID bigint NOT NULL,
-- 	img image NOT NULL DEFAULT (0x1111),
 CONSTRAINT pk_SpecObjAll_specObjID PRIMARY KEY  
(
	specObjID
)
);
CREATE TABLE HoleObj(
	holeID bigint NOT NULL,
	plateID bigint NOT NULL,
	holeType int NOT NULL,
	objType int NOT NULL,
	targetID bigint NOT NULL,
	fiberID smallint NOT NULL,
	xFocal float NOT NULL,
	yFocal float NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_HoleObj_HoleID PRIMARY KEY  
(
	holeID
)
);
CREATE TABLE SpecPhotoAll(
	specObjID bigint NOT NULL,
	mjd int NOT NULL,
	plate smallint NOT NULL,
	tile smallint NOT NULL,
	fiberID smallint NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	zConf real NOT NULL,
	zStatus smallint NOT NULL,
	specClass smallint NOT NULL,
	zWarning int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	htmID bigint NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	sciencePrimary smallint NOT NULL,
	plateID bigint NOT NULL,
	objType int NOT NULL,
	targetID bigint NOT NULL,
	targetObjID bigint NOT NULL,
	targetMode tinyint NOT NULL,
	objID bigint NULL,
	skyVersion int NULL,
	run int NULL,
	rerun int NULL,
	camcol int NULL,
	field int NULL,
	obj int NULL,
	mode int NULL,
	nChild int NULL,
	type int NULL,
	flags bigint NULL,
	psfMag_u real NULL,
	psfMag_g real NULL,
	psfMag_r real NULL,
	psfMag_i real NULL,
	psfMag_z real NULL,
	psfMagErr_u real NULL,
	psfMagErr_g real NULL,
	psfMagErr_r real NULL,
	psfMagErr_i real NULL,
	psfMagErr_z real NULL,
	fiberMag_u real NULL,
	fiberMag_g real NULL,
	fiberMag_r real NULL,
	fiberMag_i real NULL,
	fiberMag_z real NULL,
	fiberMagErr_u real NULL,
	fiberMagErr_g real NULL,
	fiberMagErr_r real NULL,
	fiberMagErr_i real NULL,
	fiberMagErr_z real NULL,
	petroMag_u real NULL,
	petroMag_g real NULL,
	petroMag_r real NULL,
	petroMag_i real NULL,
	petroMag_z real NULL,
	petroMagErr_u real NULL,
	petroMagErr_g real NULL,
	petroMagErr_r real NULL,
	petroMagErr_i real NULL,
	petroMagErr_z real NULL,
	modelMag_u real NULL,
	modelMag_g real NULL,
	modelMag_r real NULL,
	modelMag_i real NULL,
	modelMag_z real NULL,
	modelMagErr_u real NULL,
	modelMagErr_g real NULL,
	modelMagErr_r real NULL,
	modelMagErr_i real NULL,
	modelMagErr_z real NULL,
	mRrCc_r real NULL,
	mRrCcErr_r real NULL,
	status int NULL,
	photoRa float NULL,
	photoDec float NULL,
	extinction_u real NULL,
	extinction_g real NULL,
	extinction_r real NULL,
	extinction_i real NULL,
	extinction_z real NULL,
	bestPrimTarget int NULL,
	bestSecTarget int NULL,
	priority int NULL,
	fieldID bigint NULL,
	dered_u real NULL,
	dered_g real NULL,
	dered_r real NULL,
	dered_i real NULL,
	dered_z real NULL,
	size real NULL,
	tileRun smallint NULL,
	tiPriority int NULL,
	tiMask int NULL,
	collisionGroupID int NULL,
 CONSTRAINT pk_SpecPhotoAll_specObjID PRIMARY KEY  
(
	specObjID
)
);
CREATE TABLE QuasarCatalog(
	SDSSName varchar(64) NOT NULL,
	RA float NOT NULL,
	"dec" float NOT NULL,
	z real NOT NULL,
	psfmag_u real NOT NULL,
	psfmagerr_u real NOT NULL,
	psfmag_g real NOT NULL,
	psfmagerr_g real NOT NULL,
	psfmag_r real NOT NULL,
	psfmagerr_r real NOT NULL,
	psfmag_i real NOT NULL,
	psfmagerr_i real NOT NULL,
	psfmag_z real NOT NULL,
	psfmagerr_z real NOT NULL,
	A_u real NOT NULL,
	lgNH real NOT NULL,
	FIRSTmag real NOT NULL,
	FIRST_sn real NOT NULL,
	FIRST_sep real NOT NULL,
	FIRST_flux_at_opt smallint NOT NULL,
	FIRST_extmatch smallint NOT NULL,
	lg_RASS_rate real NOT NULL,
	RASS_sn real NOT NULL,
	RASS_sep real NOT NULL,
	TWOMASSmag_J real NOT NULL,
	TWOMASSmagerr_J real NOT NULL,
	TWOMASSmag_H real NOT NULL,
	TWOMASSmagerr_H real NOT NULL,
	TWOMASSmag_K real NOT NULL,
	TWOMASSmagerr_K real NOT NULL,
	TWOMASS_sep real NOT NULL,
	M_i real NOT NULL,
	morphology smallint NOT NULL,
	scienceprimary smallint NOT NULL,
	mode smallint NOT NULL,
	bestprimtarget int NOT NULL,
	ts_b_qso smallint NOT NULL,
	ts_b_hiz smallint NOT NULL,
	ts_b_FIRST smallint NOT NULL,
	ts_b_ROSAT smallint NOT NULL,
	ts_b_serendip smallint NOT NULL,
	ts_b_star smallint NOT NULL,
	ts_b_gal smallint NOT NULL,
	run_best smallint NOT NULL,
	mjd_best int NOT NULL,
	mjd_spec int NOT NULL,
	plate smallint NOT NULL,
	fiberid smallint NOT NULL,
	rerun_best smallint NOT NULL,
	camcol_best smallint NOT NULL,
	field_best smallint NOT NULL,
	obj_best smallint NOT NULL,
	tilingchunk smallint NOT NULL,
	targprimtarget int NOT NULL,
	t_psfmag_u real NOT NULL,
	t_psfmagerr_u real NOT NULL,
	t_psfmag_g real NOT NULL,
	t_psfmagerr_g real NOT NULL,
	t_psfmag_r real NOT NULL,
	t_psfmagerr_r real NOT NULL,
	t_psfmag_i real NOT NULL,
	t_psfmagerr_i real NOT NULL,
	t_psfmag_z real NOT NULL,
	t_psfmagerr_z real NOT NULL,
	object_name varchar(64) NULL,
	specObjId bigint NOT NULL,
 CONSTRAINT pk_QuasarCatalog_SpecObjID PRIMARY KEY  
(
	specObjId
)
);
CREATE TABLE SpecLineIndex(
	specLineIndexID bigint NOT NULL,
	name varchar(16) NOT NULL,
	ew real NOT NULL,
	ewErr real NOT NULL,
	mag real NOT NULL,
	magErr real NOT NULL,
	waveMin real NOT NULL,
	waveMax real NOT NULL,
	z real NOT NULL,
	sn real NOT NULL,
	specobjID bigint NOT NULL,
 CONSTRAINT pk_SpecLineIndex_specLineIndexID PRIMARY KEY  
(
	specLineIndexID
)
);
CREATE TABLE SpecLineAll(
	specLineID bigint NOT NULL,
	wave real NOT NULL,
	waveErr real NOT NULL,
	waveMin real NOT NULL,
	waveMax real NOT NULL,
	sigma real NOT NULL,
	sigmaErr real NOT NULL,
	sigmaMin real NOT NULL,
	sigmaMax real NOT NULL,
	height real NOT NULL,
	heightErr real NOT NULL,
	continuum real NOT NULL,
	ew real NOT NULL,
	ewErr real NOT NULL,
	ewMin real NOT NULL,
	specIndex real NOT NULL,
	nSigma real NOT NULL,
	chisq real NOT NULL,
	nu smallint NOT NULL,
	restWave real NOT NULL,
	lineID int NOT NULL,
	weight real NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	lineMask int NOT NULL,
	category smallint NOT NULL,
	specobjID bigint NOT NULL,
 CONSTRAINT pk_SpecLineAll_specLineID PRIMARY KEY  
(
	specLineID
)
);
CREATE TABLE XCRedshift(
	xcRedShiftID bigint NOT NULL,
	tempNo int NOT NULL,
	peakNo int NOT NULL,
	shift real NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	r real NOT NULL,
	zConf real NOT NULL,
	height real NOT NULL,
	width real NOT NULL,
	widthErr real NOT NULL,
	delta real NOT NULL,
	rms real NOT NULL,
	specObjID bigint NOT NULL,
 CONSTRAINT pk_XCRedshift_xcRedshiftID PRIMARY KEY  
(
	xcRedShiftID
)
);
CREATE TABLE ELRedShift(
	elRedShiftID bigint NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	zConf real NOT NULL,
	nFit int NOT NULL,
	wtFit real NOT NULL,
	specObjID bigint NOT NULL,
 CONSTRAINT pk_ELRedShift_elRedShiftID PRIMARY KEY  
(
	elRedShiftID
)
);
CREATE TABLE TargetInfo(
	targetObjID bigint NOT NULL,
	targetID bigint NOT NULL,
	skyVersion int NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	priority int NOT NULL,
	programType int NOT NULL,
	programName varchar(32) NOT NULL,
	targetMode tinyint NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TargetInfo_skyVersion_targetI PRIMARY KEY  
(
	skyVersion,
	targetID
)
);
CREATE TABLE TiledTargetAll(
	tile smallint NOT NULL,
	targetID bigint NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	htmID bigint NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	objType int NOT NULL,
	duplicate tinyint NOT NULL,
	untiled tinyint NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TiledTargetAll_targetID_tile PRIMARY KEY  
(
	targetID,
	tile
)
);
CREATE TABLE PlateX(
	plateID bigint NOT NULL,
	spRerun int NOT NULL,
	mjd int NOT NULL,
	plate smallint NOT NULL,
	tile smallint NOT NULL,
	mapMjd int NOT NULL,
	nExp smallint NOT NULL,
	tai real NOT NULL,
	raBoresight float NOT NULL,
	decBoresight float NOT NULL,
	taiHMS varchar(64) NOT NULL,
	expTime real NOT NULL,
	expTimeB1 real NOT NULL,
	expTimeB2 real NOT NULL,
	expTimeR1 real NOT NULL,
	expTimeR2 real NOT NULL,
	helioRV real NOT NULL,
	ra real NOT NULL,
	"dec" real NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	htmID bigint NOT NULL,
	sn1_0 real NOT NULL,
	sn1_1 real NOT NULL,
	sn1_2 real NOT NULL,
	sn2_0 real NOT NULL,
	sn2_1 real NOT NULL,
	sn2_2 real NOT NULL,
	dateObs varchar(12) NOT NULL,
	timeSys varchar(8) NOT NULL,
	quality varchar(12) NOT NULL,
	name varchar(32) NOT NULL,
	program varchar(16) NOT NULL,
	version varchar(64) NOT NULL,
	observer varchar(64) NOT NULL,
	camVer varchar(64) NOT NULL,
	spec2DVer varchar(64) NOT NULL,
	utilsVer varchar(64) NOT NULL,
	spec1DVer varchar(64) NOT NULL,
	readVer varchar(64) NOT NULL,
	combVer varchar(64) NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	rOffset1 real NOT NULL,
	rSigma1 real NOT NULL,
	grOff1 real NOT NULL,
	grSigma1 real NOT NULL,
	rOffset2 real NOT NULL,
	rSigma2 real NOT NULL,
	grOff2 real NOT NULL,
	grSigma2 real NOT NULL,
	sfd_used tinyint NOT NULL,
	xygrSig1 real NOT NULL,
	xygrSig2 real NOT NULL,
	mpgrSig1 real NOT NULL,
	mpgrSig2 real NOT NULL,
	mpgrOff1 real NOT NULL,
	mpgrOff2 real NOT NULL,
	isPrimary tinyint NOT NULL,
	cartridgeID smallint NOT NULL,
	plateVersion varchar(32) NOT NULL,
	haMin real NOT NULL,
	haMax real NOT NULL,
	mjdDesign int NOT NULL,
	theta real NOT NULL,
	fscanVersion varchar(32) NOT NULL,
	fmapVersion varchar(32) NOT NULL,
	fscanMode varchar(32) NOT NULL,
	fscanSpeed int NOT NULL,
	programType int NOT NULL,
	programName varchar(32) NOT NULL,
	loadVersion int NOT NULL,
	expID blob NULL,
 CONSTRAINT pk_PlateX_plateID PRIMARY KEY  
(
	plateID
)
);
CREATE TABLE Sector2Tile(
	regionID bigint NOT NULL,
	type varchar(16) NOT NULL,
	tile smallint NOT NULL,
	isMask int NOT NULL,
 CONSTRAINT pk_Sector2Tile_regionId_tile PRIMARY KEY  
(
	regionID,
	tile
)
);
CREATE TABLE TilingInfo(
	tileRun smallint NOT NULL,
	tid int NOT NULL,
	tile smallint NOT NULL,
	tiPriority int NOT NULL,
	tiMask int NOT NULL,
	collisionGroupID int NOT NULL,
	targetID bigint NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TilingInfo_tileRun_targetID PRIMARY KEY  
(
	tileRun,
	targetID
)
);
CREATE TABLE TileAll(
	tile smallint NOT NULL,
	tileRun smallint NOT NULL,
	raCen float NOT NULL,
	decCen float NOT NULL,
	htmID bigint NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	untiled tinyint NOT NULL,
	nTargets int NOT NULL,
	programType int NOT NULL,
	programName varchar(32) NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	completeTileVersion varchar(32) NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TileAll_tile PRIMARY KEY  
(
	tile
)
);
CREATE TABLE TilingNote(
	tilingNoteID int NOT NULL,
	tileRun smallint NOT NULL,
	tilingNote text NOT NULL,
 CONSTRAINT pk_TilingNote_tilingNoteID PRIMARY KEY  
(
	tilingNoteID
)
);
CREATE TABLE TilingGeometry(
	tilingGeometryID int NOT NULL,
	tileRun smallint NOT NULL,
	stripe int NOT NULL,
	nsbx varchar(1) NOT NULL,
	isMask tinyint NOT NULL,
	coordType int NOT NULL,
	lambdamu_0 float NOT NULL,
	lambdamu_1 float NOT NULL,
	etanu_0 float NOT NULL,
	etanu_1 float NOT NULL,
	lambdaLimit_0 float NOT NULL,
	lambdaLimit_1 float NOT NULL,
	targetVersion varchar(32) NOT NULL,
	firstArea float NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_TilingGeometry_tilingGeometry PRIMARY KEY  
(
	tilingGeometryID
)
);
CREATE TABLE RegionConvex(
	regionID bigint NOT NULL,
	convexID bigint NOT NULL,
	patch int NOT NULL,
	type varchar(16) NOT NULL,
	radius float NOT NULL DEFAULT (0),
	ra float NOT NULL DEFAULT (0),
	"dec" float NOT NULL DEFAULT (0),
	x float NOT NULL DEFAULT (0),
	y float NOT NULL DEFAULT (0),
	z float NOT NULL DEFAULT (0),
	c float NOT NULL DEFAULT (0),
	htmid bigint NOT NULL DEFAULT (0),
	convexString varchar(7900) NOT NULL DEFAULT (''),
 CONSTRAINT pk_RegionConvex_regionid_convexi PRIMARY KEY  
(
	regionID,
	convexID,
	patch
)
);
CREATE TABLE Region2Box(
	regionType varchar(16) NOT NULL,
	id bigint NOT NULL,
	boxType varchar(16) NOT NULL,
	boxid bigint NOT NULL,
 CONSTRAINT pk_Region2Box_regionType_id_boxI PRIMARY KEY  
(
	regionType,
	id,
	boxid
)
);
CREATE TABLE RegionArcs(
	regionid bigint NOT NULL,
	convexid bigint NOT NULL,
	constraintid bigint NOT NULL,
	patch int NOT NULL,
	state int NOT NULL,
	draw int NOT NULL,
	ra1 float NOT NULL,
	dec1 float NOT NULL,
	ra2 float NOT NULL,
	dec2 float NOT NULL,
	x float NOT NULL,
	y float NOT NULL,
	z float NOT NULL,
	c float NOT NULL,
	length float NOT NULL DEFAULT (0.0),
	arcid int GENERATED ALWAYS AS IDENTITY NOT NULL,
 CONSTRAINT pk_RegionArcs_regionId_convexid_ PRIMARY KEY  
(
	regionid,
	convexid,
	arcid
)
);
CREATE TABLE Sector(
	type varchar(16) NOT NULL,
	regionID bigint NOT NULL,
	nTiles int NOT NULL,
	targetVersion varchar(32) NOT NULL,
	area real NOT NULL,
 CONSTRAINT pk_Sector_regionId PRIMARY KEY  
(
	regionID
)
);
CREATE TABLE HalfSpace(
	constraintid bigint GENERATED ALWAYS AS IDENTITY NOT NULL,
	regionid bigint NOT NULL,
	convexid bigint NOT NULL,
	x float NOT NULL,
	y float NOT NULL,
	z float NOT NULL,
	c float NOT NULL,
 CONSTRAINT pk_HalfSpace_constraintid PRIMARY KEY  
(
	constraintid
)
);
CREATE TABLE Inventory(
	filename varchar(128) NOT NULL,
	name varchar(128) NOT NULL,
	type varchar(2) NOT NULL,
 CONSTRAINT pk_Inventory_filename_name PRIMARY KEY  
(
	filename,
	name
)
);
CREATE TABLE DBColumns(
	tablename varchar(128) NOT NULL,
	name varchar(64) NOT NULL,
	unit varchar(64) NOT NULL,
	ucd varchar(128) NOT NULL,
	enum varchar(64) NOT NULL,
	description varchar(2000) NOT NULL,
	rank int NOT NULL DEFAULT (0),
 CONSTRAINT pk_DBColumns_tableName_name PRIMARY KEY  
(
	tablename,
	name
)
);
CREATE TABLE DBViewCols(
	name varchar(64) NOT NULL,
	viewname varchar(128) NOT NULL,
	parent varchar(128) NOT NULL,
 CONSTRAINT pk_DBViewCols_viewName_name PRIMARY KEY  
(
	viewname,
	name
)
);
CREATE TABLE IndexMap(
	indexmapid int GENERATED ALWAYS AS IDENTITY NOT NULL,
	code varchar(2) NOT NULL,
	type varchar(32) NOT NULL,
	tableName varchar(128) NOT NULL,
	fieldList varchar(1000) NOT NULL,
	foreignKey varchar(1000) NOT NULL,
	indexgroup varchar(128) NOT NULL,
 CONSTRAINT pk_IndexMap_indexmapid PRIMARY KEY  
(
	indexmapid
)
);
CREATE TABLE BestTarget2Sector(
	objID bigint NOT NULL,
	regionID bigint NOT NULL,
	status int NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	petroMag_r real NOT NULL,
	extinction_r real NOT NULL,
 CONSTRAINT pk_BestTarget2Sector_objid_regio PRIMARY KEY  
(
	objID,
	regionID
)
);
CREATE TABLE Segment(
	segmentID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	run smallint NOT NULL,
	camCol tinyint NOT NULL,
	rerun smallint NOT NULL,
	field0 int NOT NULL,
	nFields int NOT NULL,
	dscale int NOT NULL,
	chunkID bigint NOT NULL,
	startMu int NOT NULL,
	endMu int NOT NULL,
	stripe int NOT NULL,
	strip smallint NOT NULL,
	status smallint NOT NULL,
	resolved smallint NOT NULL,
	exported smallint NOT NULL,
	families smallint NOT NULL,
	equinox float NOT NULL,
	node float NOT NULL,
	incl float NOT NULL,
	cObs float NOT NULL,
	tracking float NOT NULL,
	raMin float NOT NULL,
	raMax float NOT NULL,
	decMin float NOT NULL,
	decMax float NOT NULL,
	lMin float NOT NULL,
	lMax float NOT NULL,
	bMin float NOT NULL,
	bMax float NOT NULL,
	lambdaMin float NOT NULL,
	lambdaMax float NOT NULL,
	etaMin float NOT NULL,
	etaMax float NOT NULL,
	photoId varchar(32) NOT NULL,
	photoVersion varchar(32) NOT NULL,
	targetAstroId varchar(32) NOT NULL,
	targetAstroVersion varchar(32) NOT NULL,
	exportAstroId varchar(32) NOT NULL,
	exportAstroVersion varchar(32) NOT NULL,
	targetFCalibId varchar(32) NOT NULL,
	targetFCalibVersion varchar(32) NOT NULL,
	exportFCalibID varchar(32) NOT NULL,
	exportFCalibVersion varchar(32) NOT NULL,
	loaderVersion varchar(32) NOT NULL,
	objectSource varchar(32) NOT NULL,
	targetSource varchar(32) NOT NULL,
	targetVersion varchar(32) NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_Segment_segmentID PRIMARY KEY  
(
	segmentID
)
);
CREATE TABLE Field(
	fieldID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	nObjects smallint NOT NULL,
	nChild int NOT NULL,
	nGalaxy int NOT NULL,
	nStars int NOT NULL,
	numStars_u int NOT NULL,
	numStars_g int NOT NULL,
	numStars_r int NOT NULL,
	numStars_i int NOT NULL,
	numStars_z int NOT NULL,
	nCR_u int NOT NULL,
	nCR_g int NOT NULL,
	nCR_r int NOT NULL,
	nCR_i int NOT NULL,
	nCR_z int NOT NULL,
	nBrightObj_u int NOT NULL,
	nBrightObj_g int NOT NULL,
	nBrightObj_r int NOT NULL,
	nBrightObj_i int NOT NULL,
	nBrightObj_z int NOT NULL,
	nFaintObj_u int NOT NULL,
	nFaintObj_g int NOT NULL,
	nFaintObj_r int NOT NULL,
	nFaintObj_i int NOT NULL,
	nFaintObj_z int NOT NULL,
	quality int NOT NULL,
	mjd_u float NOT NULL,
	mjd_g float NOT NULL,
	mjd_r float NOT NULL,
	mjd_i float NOT NULL,
	mjd_z float NOT NULL,
	a_u float NOT NULL,
	a_g float NOT NULL,
	a_r float NOT NULL,
	a_i float NOT NULL,
	a_z float NOT NULL,
	b_u float NOT NULL,
	b_g float NOT NULL,
	b_r float NOT NULL,
	b_i float NOT NULL,
	b_z float NOT NULL,
	c_u float NOT NULL,
	c_g float NOT NULL,
	c_r float NOT NULL,
	c_i float NOT NULL,
	c_z float NOT NULL,
	d_u float NOT NULL,
	d_g float NOT NULL,
	d_r float NOT NULL,
	d_i float NOT NULL,
	d_z float NOT NULL,
	e_u float NOT NULL,
	e_g float NOT NULL,
	e_r float NOT NULL,
	e_i float NOT NULL,
	e_z float NOT NULL,
	f_u float NOT NULL,
	f_g float NOT NULL,
	f_r float NOT NULL,
	f_i float NOT NULL,
	f_z float NOT NULL,
	dRow0_u float NOT NULL,
	dRow0_g float NOT NULL,
	dRow0_r float NOT NULL,
	dRow0_i float NOT NULL,
	dRow0_z float NOT NULL,
	dRow1_u float NOT NULL,
	dRow1_g float NOT NULL,
	dRow1_r float NOT NULL,
	dRow1_i float NOT NULL,
	dRow1_z float NOT NULL,
	dRow2_u float NOT NULL,
	dRow2_g float NOT NULL,
	dRow2_r float NOT NULL,
	dRow2_i float NOT NULL,
	dRow2_z float NOT NULL,
	dRow3_u float NOT NULL,
	dRow3_g float NOT NULL,
	dRow3_r float NOT NULL,
	dRow3_i float NOT NULL,
	dRow3_z float NOT NULL,
	dCol0_u float NOT NULL,
	dCol0_g float NOT NULL,
	dCol0_r float NOT NULL,
	dCol0_i float NOT NULL,
	dCol0_z float NOT NULL,
	dCol1_u float NOT NULL,
	dCol1_g float NOT NULL,
	dCol1_r float NOT NULL,
	dCol1_i float NOT NULL,
	dCol1_z float NOT NULL,
	dCol2_u float NOT NULL,
	dCol2_g float NOT NULL,
	dCol2_r float NOT NULL,
	dCol2_i float NOT NULL,
	dCol2_z float NOT NULL,
	dCol3_u float NOT NULL,
	dCol3_g float NOT NULL,
	dCol3_r float NOT NULL,
	dCol3_i float NOT NULL,
	dCol3_z float NOT NULL,
	csRow_u float NOT NULL,
	csRow_g float NOT NULL,
	csRow_r float NOT NULL,
	csRow_i float NOT NULL,
	csRow_z float NOT NULL,
	csCol_u float NOT NULL,
	csCol_g float NOT NULL,
	csCol_r float NOT NULL,
	csCol_i float NOT NULL,
	csCol_z float NOT NULL,
	ccRow_u float NOT NULL,
	ccRow_g float NOT NULL,
	ccRow_r float NOT NULL,
	ccRow_i float NOT NULL,
	ccRow_z float NOT NULL,
	ccCol_u float NOT NULL,
	ccCol_g float NOT NULL,
	ccCol_r float NOT NULL,
	ccCol_i float NOT NULL,
	ccCol_z float NOT NULL,
	riCut_u float NOT NULL,
	riCut_g float NOT NULL,
	riCut_r float NOT NULL,
	riCut_i float NOT NULL,
	riCut_z float NOT NULL,
	airmass_u float NOT NULL,
	airmass_g float NOT NULL,
	airmass_r float NOT NULL,
	airmass_i float NOT NULL,
	airmass_z float NOT NULL,
	muErr_u float NOT NULL,
	muErr_g float NOT NULL,
	muErr_r float NOT NULL,
	muErr_i float NOT NULL,
	muErr_z float NOT NULL,
	nuErr_u float NOT NULL,
	nuErr_g float NOT NULL,
	nuErr_r float NOT NULL,
	nuErr_i float NOT NULL,
	nuErr_z float NOT NULL,
	raMin float NOT NULL,
	raMax float NOT NULL,
	decMin float NOT NULL,
	decMax float NOT NULL,
	scanlineArea real NOT NULL,
	stripeArea real NOT NULL,
	seqId int NOT NULL,
	aa_u real NOT NULL,
	aa_g real NOT NULL,
	aa_r real NOT NULL,
	aa_i real NOT NULL,
	aa_z real NOT NULL,
	aaErr_u real NOT NULL,
	aaErr_g real NOT NULL,
	aaErr_r real NOT NULL,
	aaErr_i real NOT NULL,
	aaErr_z real NOT NULL,
	bb_u real NOT NULL,
	bb_g real NOT NULL,
	bb_r real NOT NULL,
	bb_i real NOT NULL,
	bb_z real NOT NULL,
	bbErr_u real NOT NULL,
	bbErr_g real NOT NULL,
	bbErr_r real NOT NULL,
	bbErr_i real NOT NULL,
	bbErr_z real NOT NULL,
	cc_u real NOT NULL,
	cc_g real NOT NULL,
	cc_r real NOT NULL,
	cc_i real NOT NULL,
	cc_z real NOT NULL,
	ccErr_u real NOT NULL,
	ccErr_g real NOT NULL,
	ccErr_r real NOT NULL,
	ccErr_i real NOT NULL,
	ccErr_z real NOT NULL,
	kk_u real NOT NULL,
	kk_g real NOT NULL,
	kk_r real NOT NULL,
	kk_i real NOT NULL,
	kk_z real NOT NULL,
	kkErr_u real NOT NULL,
	kkErr_g real NOT NULL,
	kkErr_r real NOT NULL,
	kkErr_i real NOT NULL,
	kkErr_z real NOT NULL,
	framesStatus int NOT NULL,
	rowOffset_u real NOT NULL,
	rowOffset_g real NOT NULL,
	rowOffset_r real NOT NULL,
	rowOffset_i real NOT NULL,
	rowOffset_z real NOT NULL,
	colOffset_u real NOT NULL,
	colOffset_g real NOT NULL,
	colOffset_r real NOT NULL,
	colOffset_i real NOT NULL,
	colOffset_z real NOT NULL,
	saturationLevel_u int NOT NULL,
	saturationLevel_g int NOT NULL,
	saturationLevel_r int NOT NULL,
	saturationLevel_i int NOT NULL,
	saturationLevel_z int NOT NULL,
	nEffPsf_u real NOT NULL,
	nEffPsf_g real NOT NULL,
	nEffPsf_r real NOT NULL,
	nEffPsf_i real NOT NULL,
	nEffPsf_z real NOT NULL,
	skyPsp_u real NOT NULL,
	skyPsp_g real NOT NULL,
	skyPsp_r real NOT NULL,
	skyPsp_i real NOT NULL,
	skyPsp_z real NOT NULL,
	skyFrames_u real NOT NULL,
	skyFrames_g real NOT NULL,
	skyFrames_r real NOT NULL,
	skyFrames_i real NOT NULL,
	skyFrames_z real NOT NULL,
	skyFramesSub_u real NOT NULL,
	skyFramesSub_g real NOT NULL,
	skyFramesSub_r real NOT NULL,
	skyFramesSub_i real NOT NULL,
	skyFramesSub_z real NOT NULL,
	sigPix_u real NOT NULL,
	sigPix_g real NOT NULL,
	sigPix_r real NOT NULL,
	sigPix_i real NOT NULL,
	sigPix_z real NOT NULL,
	deVApCorrection_u real NOT NULL,
	deVApCorrection_g real NOT NULL,
	deVApCorrection_r real NOT NULL,
	deVApCorrection_i real NOT NULL,
	deVApCorrection_z real NOT NULL,
	deVApCorrectionErr_u real NOT NULL,
	deVApCorrectionErr_g real NOT NULL,
	deVApCorrectionErr_r real NOT NULL,
	deVApCorrectionErr_i real NOT NULL,
	deVApCorrectionErr_z real NOT NULL,
	expApCorrection_u real NOT NULL,
	expApCorrection_g real NOT NULL,
	expApCorrection_r real NOT NULL,
	expApCorrection_i real NOT NULL,
	expApCorrection_z real NOT NULL,
	expApCorrectionErr_u real NOT NULL,
	expApCorrectionErr_g real NOT NULL,
	expApCorrectionErr_r real NOT NULL,
	expApCorrectionErr_i real NOT NULL,
	expApCorrectionErr_z real NOT NULL,
	deVModelApCorrection_u real NOT NULL,
	deVModelApCorrection_g real NOT NULL,
	deVModelApCorrection_r real NOT NULL,
	deVModelApCorrection_i real NOT NULL,
	deVModelApCorrection_z real NOT NULL,
	deVModelApCorrectionErr_u real NOT NULL,
	deVModelApCorrectionErr_g real NOT NULL,
	deVModelApCorrectionErr_r real NOT NULL,
	deVModelApCorrectionErr_i real NOT NULL,
	deVModelApCorrectionErr_z real NOT NULL,
	expModelApCorrection_u real NOT NULL,
	expModelApCorrection_g real NOT NULL,
	expModelApCorrection_r real NOT NULL,
	expModelApCorrection_i real NOT NULL,
	expModelApCorrection_z real NOT NULL,
	expModelApCorrectionErr_u real NOT NULL,
	expModelApCorrectionErr_g real NOT NULL,
	expModelApCorrectionErr_r real NOT NULL,
	expModelApCorrectionErr_i real NOT NULL,
	expModelApCorrectionErr_z real NOT NULL,
	medianFiberColor_u real NOT NULL,
	medianFiberColor_g real NOT NULL,
	medianFiberColor_r real NOT NULL,
	medianFiberColor_i real NOT NULL,
	medianFiberColor_z real NOT NULL,
	medianPsfColor_u real NOT NULL,
	medianPsfColor_g real NOT NULL,
	medianPsfColor_r real NOT NULL,
	medianPsfColor_i real NOT NULL,
	medianPsfColor_z real NOT NULL,
	q_u real NOT NULL,
	q_g real NOT NULL,
	q_r real NOT NULL,
	q_i real NOT NULL,
	q_z real NOT NULL,
	u_u real NOT NULL,
	u_g real NOT NULL,
	u_r real NOT NULL,
	u_i real NOT NULL,
	u_z real NOT NULL,
	pspStatus smallint NOT NULL,
	culled int NOT NULL,
	badMask int NOT NULL,
	acceptableMask int NOT NULL,
	goodMask int NOT NULL,
	holeMask int NOT NULL,
	status_u int NOT NULL,
	status_g int NOT NULL,
	status_r int NOT NULL,
	status_i int NOT NULL,
	status_z int NOT NULL,
	sky_u real NOT NULL,
	sky_g real NOT NULL,
	sky_r real NOT NULL,
	sky_i real NOT NULL,
	sky_z real NOT NULL,
	skySig_u real NOT NULL,
	skySig_g real NOT NULL,
	skySig_r real NOT NULL,
	skySig_i real NOT NULL,
	skySig_z real NOT NULL,
	skyErr_u real NOT NULL,
	skyErr_g real NOT NULL,
	skyErr_r real NOT NULL,
	skyErr_i real NOT NULL,
	skyErr_z real NOT NULL,
	skySlope_u real NOT NULL,
	skySlope_g real NOT NULL,
	skySlope_r real NOT NULL,
	skySlope_i real NOT NULL,
	skySlope_z real NOT NULL,
	lbias_u real NOT NULL,
	lbias_g real NOT NULL,
	lbias_r real NOT NULL,
	lbias_i real NOT NULL,
	lbias_z real NOT NULL,
	rbias_u real NOT NULL,
	rbias_g real NOT NULL,
	rbias_r real NOT NULL,
	rbias_i real NOT NULL,
	rbias_z real NOT NULL,
	psfNStar_u int NOT NULL,
	psfNStar_g int NOT NULL,
	psfNStar_r int NOT NULL,
	psfNStar_i int NOT NULL,
	psfNStar_z int NOT NULL,
	psfApCorrectionErr_u real NOT NULL,
	psfApCorrectionErr_g real NOT NULL,
	psfApCorrectionErr_r real NOT NULL,
	psfApCorrectionErr_i real NOT NULL,
	psfApCorrectionErr_z real NOT NULL,
	psfSigma1_u real NOT NULL,
	psfSigma1_g real NOT NULL,
	psfSigma1_r real NOT NULL,
	psfSigma1_i real NOT NULL,
	psfSigma1_z real NOT NULL,
	psfSigma2_u real NOT NULL,
	psfSigma2_g real NOT NULL,
	psfSigma2_r real NOT NULL,
	psfSigma2_i real NOT NULL,
	psfSigma2_z real NOT NULL,
	psfB_u real NOT NULL,
	psfB_g real NOT NULL,
	psfB_r real NOT NULL,
	psfB_i real NOT NULL,
	psfB_z real NOT NULL,
	psfP0_u real NOT NULL,
	psfP0_g real NOT NULL,
	psfP0_r real NOT NULL,
	psfP0_i real NOT NULL,
	psfP0_z real NOT NULL,
	psfBeta_u real NOT NULL,
	psfBeta_g real NOT NULL,
	psfBeta_r real NOT NULL,
	psfBeta_i real NOT NULL,
	psfBeta_z real NOT NULL,
	psfSigmaP_u real NOT NULL,
	psfSigmaP_g real NOT NULL,
	psfSigmaP_r real NOT NULL,
	psfSigmaP_i real NOT NULL,
	psfSigmaP_z real NOT NULL,
	psfWidth_u real NOT NULL,
	psfWidth_g real NOT NULL,
	psfWidth_r real NOT NULL,
	psfWidth_i real NOT NULL,
	psfWidth_z real NOT NULL,
	psfPsfCounts_u real NOT NULL,
	psfPsfCounts_g real NOT NULL,
	psfPsfCounts_r real NOT NULL,
	psfPsfCounts_i real NOT NULL,
	psfPsfCounts_z real NOT NULL,
	psf2GSigma1_u real NOT NULL,
	psf2GSigma1_g real NOT NULL,
	psf2GSigma1_r real NOT NULL,
	psf2GSigma1_i real NOT NULL,
	psf2GSigma1_z real NOT NULL,
	psf2GSigma2_u real NOT NULL,
	psf2GSigma2_g real NOT NULL,
	psf2GSigma2_r real NOT NULL,
	psf2GSigma2_i real NOT NULL,
	psf2GSigma2_z real NOT NULL,
	psf2GB_u real NOT NULL,
	psf2GB_g real NOT NULL,
	psf2GB_r real NOT NULL,
	psf2GB_i real NOT NULL,
	psf2GB_z real NOT NULL,
	psfCounts_u real NOT NULL,
	psfCounts_g real NOT NULL,
	psfCounts_r real NOT NULL,
	psfCounts_i real NOT NULL,
	psfCounts_z real NOT NULL,
	gain_u real NOT NULL,
	gain_g real NOT NULL,
	gain_r real NOT NULL,
	gain_i real NOT NULL,
	gain_z real NOT NULL,
	darkVariance_u real NOT NULL,
	darkVariance_g real NOT NULL,
	darkVariance_r real NOT NULL,
	darkVariance_i real NOT NULL,
	darkVariance_z real NOT NULL,
	nProf_u int NOT NULL,
	nProf_g int NOT NULL,
	nProf_r int NOT NULL,
	nProf_i int NOT NULL,
	nProf_z int NOT NULL,
	loadVersion int NOT NULL,
	segmentID bigint NOT NULL,
 CONSTRAINT pk_Field_fieldID PRIMARY KEY  
(
	fieldID
)
);
CREATE TABLE PhotoObjAll(
	objID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	obj smallint NOT NULL,
	mode tinyint NOT NULL,
	nChild smallint NOT NULL,
	type smallint NOT NULL,
	probPSF real NOT NULL,
	insideMask tinyint NOT NULL,
	flags bigint NOT NULL,
	rowc real NOT NULL,
	rowcErr real NOT NULL,
	colc real NOT NULL,
	colcErr real NOT NULL,
	rowv real NOT NULL,
	rowvErr real NOT NULL,
	colv real NOT NULL,
	colvErr real NOT NULL,
	rowc_u real NOT NULL,
	rowc_g real NOT NULL,
	rowc_r real NOT NULL,
	rowc_i real NOT NULL,
	rowc_z real NOT NULL,
	rowcErr_u real NOT NULL,
	rowcErr_g real NOT NULL,
	rowcErr_r real NOT NULL,
	rowcErr_i real NOT NULL,
	rowcErr_z real NOT NULL,
	colc_u real NOT NULL,
	colc_g real NOT NULL,
	colc_r real NOT NULL,
	colc_i real NOT NULL,
	colc_z real NOT NULL,
	colcErr_u real NOT NULL,
	colcErr_g real NOT NULL,
	colcErr_r real NOT NULL,
	colcErr_i real NOT NULL,
	colcErr_z real NOT NULL,
	sky_u real NOT NULL,
	sky_g real NOT NULL,
	sky_r real NOT NULL,
	sky_i real NOT NULL,
	sky_z real NOT NULL,
	skyErr_u real NOT NULL,
	skyErr_g real NOT NULL,
	skyErr_r real NOT NULL,
	skyErr_i real NOT NULL,
	skyErr_z real NOT NULL,
	psfMag_u real NOT NULL,
	psfMag_g real NOT NULL,
	psfMag_r real NOT NULL,
	psfMag_i real NOT NULL,
	psfMag_z real NOT NULL,
	psfMagErr_u real NOT NULL,
	psfMagErr_g real NOT NULL,
	psfMagErr_r real NOT NULL,
	psfMagErr_i real NOT NULL,
	psfMagErr_z real NOT NULL,
	fiberMag_u real NOT NULL,
	fiberMag_g real NOT NULL,
	fiberMag_r real NOT NULL,
	fiberMag_i real NOT NULL,
	fiberMag_z real NOT NULL,
	fiberMagErr_u real NOT NULL,
	fiberMagErr_g real NOT NULL,
	fiberMagErr_r real NOT NULL,
	fiberMagErr_i real NOT NULL,
	fiberMagErr_z real NOT NULL,
	petroMag_u real NOT NULL,
	petroMag_g real NOT NULL,
	petroMag_r real NOT NULL,
	petroMag_i real NOT NULL,
	petroMag_z real NOT NULL,
	petroMagErr_u real NOT NULL,
	petroMagErr_g real NOT NULL,
	petroMagErr_r real NOT NULL,
	petroMagErr_i real NOT NULL,
	petroMagErr_z real NOT NULL,
	petroRad_u real NOT NULL,
	petroRad_g real NOT NULL,
	petroRad_r real NOT NULL,
	petroRad_i real NOT NULL,
	petroRad_z real NOT NULL,
	petroRadErr_u real NOT NULL,
	petroRadErr_g real NOT NULL,
	petroRadErr_r real NOT NULL,
	petroRadErr_i real NOT NULL,
	petroRadErr_z real NOT NULL,
	petroR50_u real NOT NULL,
	petroR50_g real NOT NULL,
	petroR50_r real NOT NULL,
	petroR50_i real NOT NULL,
	petroR50_z real NOT NULL,
	petroR50Err_u real NOT NULL,
	petroR50Err_g real NOT NULL,
	petroR50Err_r real NOT NULL,
	petroR50Err_i real NOT NULL,
	petroR50Err_z real NOT NULL,
	petroR90_u real NOT NULL,
	petroR90_g real NOT NULL,
	petroR90_r real NOT NULL,
	petroR90_i real NOT NULL,
	petroR90_z real NOT NULL,
	petroR90Err_u real NOT NULL,
	petroR90Err_g real NOT NULL,
	petroR90Err_r real NOT NULL,
	petroR90Err_i real NOT NULL,
	petroR90Err_z real NOT NULL,
	q_u real NOT NULL,
	q_g real NOT NULL,
	q_r real NOT NULL,
	q_i real NOT NULL,
	q_z real NOT NULL,
	qErr_u real NOT NULL,
	qErr_g real NOT NULL,
	qErr_r real NOT NULL,
	qErr_i real NOT NULL,
	qErr_z real NOT NULL,
	u_u real NOT NULL,
	u_g real NOT NULL,
	u_r real NOT NULL,
	u_i real NOT NULL,
	u_z real NOT NULL,
	uErr_u real NOT NULL,
	uErr_g real NOT NULL,
	uErr_r real NOT NULL,
	uErr_i real NOT NULL,
	uErr_z real NOT NULL,
	mE1_u real NOT NULL,
	mE1_g real NOT NULL,
	mE1_r real NOT NULL,
	mE1_i real NOT NULL,
	mE1_z real NOT NULL,
	mE2_u real NOT NULL,
	mE2_g real NOT NULL,
	mE2_r real NOT NULL,
	mE2_i real NOT NULL,
	mE2_z real NOT NULL,
	mE1E1Err_u real NOT NULL,
	mE1E1Err_g real NOT NULL,
	mE1E1Err_r real NOT NULL,
	mE1E1Err_i real NOT NULL,
	mE1E1Err_z real NOT NULL,
	mE1E2Err_u real NOT NULL,
	mE1E2Err_g real NOT NULL,
	mE1E2Err_r real NOT NULL,
	mE1E2Err_i real NOT NULL,
	mE1E2Err_z real NOT NULL,
	mE2E2Err_u real NOT NULL,
	mE2E2Err_g real NOT NULL,
	mE2E2Err_r real NOT NULL,
	mE2E2Err_i real NOT NULL,
	mE2E2Err_z real NOT NULL,
	mRrCc_u real NOT NULL,
	mRrCc_g real NOT NULL,
	mRrCc_r real NOT NULL,
	mRrCc_i real NOT NULL,
	mRrCc_z real NOT NULL,
	mRrCcErr_u real NOT NULL,
	mRrCcErr_g real NOT NULL,
	mRrCcErr_r real NOT NULL,
	mRrCcErr_i real NOT NULL,
	mRrCcErr_z real NOT NULL,
	mCr4_u real NOT NULL,
	mCr4_g real NOT NULL,
	mCr4_r real NOT NULL,
	mCr4_i real NOT NULL,
	mCr4_z real NOT NULL,
	mE1PSF_u real NOT NULL,
	mE1PSF_g real NOT NULL,
	mE1PSF_r real NOT NULL,
	mE1PSF_i real NOT NULL,
	mE1PSF_z real NOT NULL,
	mE2PSF_u real NOT NULL,
	mE2PSF_g real NOT NULL,
	mE2PSF_r real NOT NULL,
	mE2PSF_i real NOT NULL,
	mE2PSF_z real NOT NULL,
	mRrCcPSF_u real NOT NULL,
	mRrCcPSF_g real NOT NULL,
	mRrCcPSF_r real NOT NULL,
	mRrCcPSF_i real NOT NULL,
	mRrCcPSF_z real NOT NULL,
	mCr4PSF_u real NOT NULL,
	mCr4PSF_g real NOT NULL,
	mCr4PSF_r real NOT NULL,
	mCr4PSF_i real NOT NULL,
	mCr4PSF_z real NOT NULL,
	isoRowc_u real NOT NULL,
	isoRowc_g real NOT NULL,
	isoRowc_r real NOT NULL,
	isoRowc_i real NOT NULL,
	isoRowc_z real NOT NULL,
	isoRowcErr_u real NOT NULL,
	isoRowcErr_g real NOT NULL,
	isoRowcErr_r real NOT NULL,
	isoRowcErr_i real NOT NULL,
	isoRowcErr_z real NOT NULL,
	isoRowcGrad_u real NOT NULL,
	isoRowcGrad_g real NOT NULL,
	isoRowcGrad_r real NOT NULL,
	isoRowcGrad_i real NOT NULL,
	isoRowcGrad_z real NOT NULL,
	isoColc_u real NOT NULL,
	isoColc_g real NOT NULL,
	isoColc_r real NOT NULL,
	isoColc_i real NOT NULL,
	isoColc_z real NOT NULL,
	isoColcErr_u real NOT NULL,
	isoColcErr_g real NOT NULL,
	isoColcErr_r real NOT NULL,
	isoColcErr_i real NOT NULL,
	isoColcErr_z real NOT NULL,
	isoColcGrad_u real NOT NULL,
	isoColcGrad_g real NOT NULL,
	isoColcGrad_r real NOT NULL,
	isoColcGrad_i real NOT NULL,
	isoColcGrad_z real NOT NULL,
	isoA_u real NOT NULL,
	isoA_g real NOT NULL,
	isoA_r real NOT NULL,
	isoA_i real NOT NULL,
	isoA_z real NOT NULL,
	isoAErr_u real NOT NULL,
	isoAErr_g real NOT NULL,
	isoAErr_r real NOT NULL,
	isoAErr_i real NOT NULL,
	isoAErr_z real NOT NULL,
	isoB_u real NOT NULL,
	isoB_g real NOT NULL,
	isoB_r real NOT NULL,
	isoB_i real NOT NULL,
	isoB_z real NOT NULL,
	isoBErr_u real NOT NULL,
	isoBErr_g real NOT NULL,
	isoBErr_r real NOT NULL,
	isoBErr_i real NOT NULL,
	isoBErr_z real NOT NULL,
	isoAGrad_u real NOT NULL,
	isoAGrad_g real NOT NULL,
	isoAGrad_r real NOT NULL,
	isoAGrad_i real NOT NULL,
	isoAGrad_z real NOT NULL,
	isoBGrad_u real NOT NULL,
	isoBGrad_g real NOT NULL,
	isoBGrad_r real NOT NULL,
	isoBGrad_i real NOT NULL,
	isoBGrad_z real NOT NULL,
	isoPhi_u real NOT NULL,
	isoPhi_g real NOT NULL,
	isoPhi_r real NOT NULL,
	isoPhi_i real NOT NULL,
	isoPhi_z real NOT NULL,
	isoPhiErr_u real NOT NULL,
	isoPhiErr_g real NOT NULL,
	isoPhiErr_r real NOT NULL,
	isoPhiErr_i real NOT NULL,
	isoPhiErr_z real NOT NULL,
	isoPhiGrad_u real NOT NULL,
	isoPhiGrad_g real NOT NULL,
	isoPhiGrad_r real NOT NULL,
	isoPhiGrad_i real NOT NULL,
	isoPhiGrad_z real NOT NULL,
	deVRad_u real NOT NULL,
	deVRad_g real NOT NULL,
	deVRad_r real NOT NULL,
	deVRad_i real NOT NULL,
	deVRad_z real NOT NULL,
	deVRadErr_u real NOT NULL,
	deVRadErr_g real NOT NULL,
	deVRadErr_r real NOT NULL,
	deVRadErr_i real NOT NULL,
	deVRadErr_z real NOT NULL,
	deVAB_u real NOT NULL,
	deVAB_g real NOT NULL,
	deVAB_r real NOT NULL,
	deVAB_i real NOT NULL,
	deVAB_z real NOT NULL,
	deVABErr_u real NOT NULL,
	deVABErr_g real NOT NULL,
	deVABErr_r real NOT NULL,
	deVABErr_i real NOT NULL,
	deVABErr_z real NOT NULL,
	deVPhi_u real NOT NULL,
	deVPhi_g real NOT NULL,
	deVPhi_r real NOT NULL,
	deVPhi_i real NOT NULL,
	deVPhi_z real NOT NULL,
	deVPhiErr_u real NOT NULL,
	deVPhiErr_g real NOT NULL,
	deVPhiErr_r real NOT NULL,
	deVPhiErr_i real NOT NULL,
	deVPhiErr_z real NOT NULL,
	deVMag_u real NOT NULL,
	deVMag_g real NOT NULL,
	deVMag_r real NOT NULL,
	deVMag_i real NOT NULL,
	deVMag_z real NOT NULL,
	deVMagErr_u real NOT NULL,
	deVMagErr_g real NOT NULL,
	deVMagErr_r real NOT NULL,
	deVMagErr_i real NOT NULL,
	deVMagErr_z real NOT NULL,
	expRad_u real NOT NULL,
	expRad_g real NOT NULL,
	expRad_r real NOT NULL,
	expRad_i real NOT NULL,
	expRad_z real NOT NULL,
	expRadErr_u real NOT NULL,
	expRadErr_g real NOT NULL,
	expRadErr_r real NOT NULL,
	expRadErr_i real NOT NULL,
	expRadErr_z real NOT NULL,
	expAB_u real NOT NULL,
	expAB_g real NOT NULL,
	expAB_r real NOT NULL,
	expAB_i real NOT NULL,
	expAB_z real NOT NULL,
	expABErr_u real NOT NULL,
	expABErr_g real NOT NULL,
	expABErr_r real NOT NULL,
	expABErr_i real NOT NULL,
	expABErr_z real NOT NULL,
	expPhi_u real NOT NULL,
	expPhi_g real NOT NULL,
	expPhi_r real NOT NULL,
	expPhi_i real NOT NULL,
	expPhi_z real NOT NULL,
	expPhiErr_u real NOT NULL,
	expPhiErr_g real NOT NULL,
	expPhiErr_r real NOT NULL,
	expPhiErr_i real NOT NULL,
	expPhiErr_z real NOT NULL,
	expMag_u real NOT NULL,
	expMag_g real NOT NULL,
	expMag_r real NOT NULL,
	expMag_i real NOT NULL,
	expMag_z real NOT NULL,
	expMagErr_u real NOT NULL,
	expMagErr_g real NOT NULL,
	expMagErr_r real NOT NULL,
	expMagErr_i real NOT NULL,
	expMagErr_z real NOT NULL,
	modelMag_u real NOT NULL,
	modelMag_g real NOT NULL,
	modelMag_r real NOT NULL,
	modelMag_i real NOT NULL,
	modelMag_z real NOT NULL,
	modelMagErr_u real NOT NULL,
	modelMagErr_g real NOT NULL,
	modelMagErr_r real NOT NULL,
	modelMagErr_i real NOT NULL,
	modelMagErr_z real NOT NULL,
	texture_u real NOT NULL,
	texture_g real NOT NULL,
	texture_r real NOT NULL,
	texture_i real NOT NULL,
	texture_z real NOT NULL,
	lnLStar_u real NOT NULL,
	lnLStar_g real NOT NULL,
	lnLStar_r real NOT NULL,
	lnLStar_i real NOT NULL,
	lnLStar_z real NOT NULL,
	lnLExp_u real NOT NULL,
	lnLExp_g real NOT NULL,
	lnLExp_r real NOT NULL,
	lnLExp_i real NOT NULL,
	lnLExp_z real NOT NULL,
	lnLDeV_u real NOT NULL,
	lnLDeV_g real NOT NULL,
	lnLDeV_r real NOT NULL,
	lnLDeV_i real NOT NULL,
	lnLDeV_z real NOT NULL,
	fracDeV_u real NOT NULL,
	fracDeV_g real NOT NULL,
	fracDeV_r real NOT NULL,
	fracDeV_i real NOT NULL,
	fracDeV_z real NOT NULL,
	flags_u bigint NOT NULL,
	flags_g bigint NOT NULL,
	flags_r bigint NOT NULL,
	flags_i bigint NOT NULL,
	flags_z bigint NOT NULL,
	type_u int NOT NULL,
	type_g int NOT NULL,
	type_r int NOT NULL,
	type_i int NOT NULL,
	type_z int NOT NULL,
	probPSF_u real NOT NULL,
	probPSF_g real NOT NULL,
	probPSF_r real NOT NULL,
	probPSF_i real NOT NULL,
	probPSF_z real NOT NULL,
	status int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	offsetRa_u real NOT NULL,
	offsetRa_g real NOT NULL,
	offsetRa_r real NOT NULL,
	offsetRa_i real NOT NULL,
	offsetRa_z real NOT NULL,
	offsetDec_u real NOT NULL,
	offsetDec_g real NOT NULL,
	offsetDec_r real NOT NULL,
	offsetDec_i real NOT NULL,
	offsetDec_z real NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	priority int NOT NULL,
	rho real NOT NULL,
	nProf_u int NOT NULL,
	nProf_g int NOT NULL,
	nProf_r int NOT NULL,
	nProf_i int NOT NULL,
	nProf_z int NOT NULL,
	loadVersion int NOT NULL,
	htmID bigint NOT NULL,
	fieldID bigint NOT NULL,
	parentID bigint NOT NULL DEFAULT (0),
	specObjID bigint NOT NULL DEFAULT (0),
	u real NOT NULL DEFAULT ((-9999)),
	g real NOT NULL DEFAULT ((-9999)),
	r real NOT NULL DEFAULT ((-9999)),
	i real NOT NULL DEFAULT ((-9999)),
	z real NOT NULL DEFAULT ((-9999)),
	err_u real NOT NULL DEFAULT ((-9999)),
	err_g real NOT NULL DEFAULT ((-9999)),
	err_r real NOT NULL DEFAULT ((-9999)),
	err_i real NOT NULL DEFAULT ((-9999)),
	err_z real NOT NULL DEFAULT ((-9999)),
	dered_u real NOT NULL DEFAULT ((-9999)),
	dered_g real NOT NULL DEFAULT ((-9999)),
	dered_r real NOT NULL DEFAULT ((-9999)),
	dered_i real NOT NULL DEFAULT ((-9999)),
	dered_z real NOT NULL DEFAULT ((-9999)),
 CONSTRAINT pk_PhotoObjAll_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE PhotoTag(
	objID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	obj smallint NOT NULL,
	mode tinyint NOT NULL,
	nChild smallint NOT NULL,
	type smallint NOT NULL,
	probPSF real NOT NULL,
	insideMask tinyint NOT NULL,
	flags bigint NOT NULL,
	psfMag_u real NOT NULL,
	psfMag_g real NOT NULL,
	psfMag_r real NOT NULL,
	psfMag_i real NOT NULL,
	psfMag_z real NOT NULL,
	psfMagErr_u real NOT NULL,
	psfMagErr_g real NOT NULL,
	psfMagErr_r real NOT NULL,
	psfMagErr_i real NOT NULL,
	psfMagErr_z real NOT NULL,
	petroMag_u real NOT NULL,
	petroMag_g real NOT NULL,
	petroMag_r real NOT NULL,
	petroMag_i real NOT NULL,
	petroMag_z real NOT NULL,
	petroMagErr_u real NOT NULL,
	petroMagErr_g real NOT NULL,
	petroMagErr_r real NOT NULL,
	petroMagErr_i real NOT NULL,
	petroMagErr_z real NOT NULL,
	petroR50_r real NOT NULL,
	petroR90_r real NOT NULL,
	modelMag_u real NOT NULL,
	modelMag_g real NOT NULL,
	modelMag_r real NOT NULL,
	modelMag_i real NOT NULL,
	modelMag_z real NOT NULL,
	modelMagErr_u real NOT NULL,
	modelMagErr_g real NOT NULL,
	modelMagErr_r real NOT NULL,
	modelMagErr_i real NOT NULL,
	modelMagErr_z real NOT NULL,
	mRrCc_r real NOT NULL,
	mRrCcErr_r real NOT NULL,
	lnLStar_r real NOT NULL,
	lnLExp_r real NOT NULL,
	lnLDeV_r real NOT NULL,
	status int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	primTarget int NOT NULL,
	secTarget int NOT NULL,
	extinction_u real NOT NULL,
	extinction_g real NOT NULL,
	extinction_r real NOT NULL,
	extinction_i real NOT NULL,
	extinction_z real NOT NULL,
	htmID bigint NOT NULL,
	fieldID bigint NOT NULL,
	specObjID bigint NOT NULL DEFAULT (0),
	size real NOT NULL DEFAULT ((-9999)),
 CONSTRAINT pk_PhotoTag_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE Frame(
	fieldID bigint NOT NULL,
	zoom int NOT NULL,
	run int NOT NULL,
	rerun int NOT NULL,
	camcol int NOT NULL,
	field int NOT NULL,
	stripe int NOT NULL,
	strip int NOT NULL,
	a float NOT NULL,
	b float NOT NULL,
	c float NOT NULL,
	d float NOT NULL,
	e float NOT NULL,
	f float NOT NULL,
	node float NOT NULL,
	incl float NOT NULL,
	raMin float NOT NULL,
	raMax float NOT NULL,
	decMin float NOT NULL,
	decMax float NOT NULL,
	mu float NOT NULL DEFAULT (0),
	nu float NOT NULL DEFAULT (0),
	ra float NOT NULL DEFAULT (0),
	"dec" float NOT NULL DEFAULT (0),
	cx float NOT NULL DEFAULT (0),
	cy float NOT NULL DEFAULT (0),
	cz float NOT NULL DEFAULT (0),
	htmID bigint NOT NULL DEFAULT (0),
-- 	img image NOT NULL DEFAULT (0x1111),
 CONSTRAINT pk_Frame_fieldID_zoom PRIMARY KEY  
(
	fieldID,
	zoom
)
);
CREATE TABLE RunQA(
	fieldID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	first tinyint NOT NULL,
	run smallint NOT NULL,
	rerun smallint NOT NULL,
	camcol tinyint NOT NULL,
	field smallint NOT NULL,
	fieldQall tinyint NOT NULL,
	opdbq tinyint NOT NULL,
	appsfq tinyint NOT NULL,
	pcmedq tinyint NOT NULL,
	pcrmsq tinyint NOT NULL,
	PSFwcondq tinyint NOT NULL,
	pc_s real NOT NULL,
	pc_w real NOT NULL,
	pc_x real NOT NULL,
	pc_y real NOT NULL,
	rmsPC_s real NOT NULL,
	rmsPC_w real NOT NULL,
	rmsPC_x real NOT NULL,
	rmsPC_y real NOT NULL,
	medAp7mPSF_u real NOT NULL,
	medAp7mPSF_g real NOT NULL,
	medAp7mPSF_r real NOT NULL,
	medAp7mPSF_i real NOT NULL,
	medAp7mPSF_z real NOT NULL,
	rmsAp7mPSF_u real NOT NULL,
	rmsAp7mPSF_g real NOT NULL,
	rmsAp7mPSF_r real NOT NULL,
	rmsAp7mPSF_i real NOT NULL,
	rmsAp7mPSF_z real NOT NULL,
	seeing_u real NOT NULL,
	seeing_g real NOT NULL,
	seeing_r real NOT NULL,
	seeing_i real NOT NULL,
	seeing_z real NOT NULL,
 CONSTRAINT pk_RunQA_fieldID PRIMARY KEY  
(
	fieldID
)
);
CREATE TABLE FieldProfile(
	bin tinyint NOT NULL,
	band tinyint NOT NULL,
	profMean real NOT NULL,
	profMed real NOT NULL,
	profSig real NOT NULL,
	fieldID bigint NOT NULL,
 CONSTRAINT pk_FieldProfile_fieldID_bin_band PRIMARY KEY  
(
	fieldID,
	bin,
	band
)
);
CREATE TABLE PhotoProfile(
	bin tinyint NOT NULL,
	band tinyint NOT NULL,
	profMean real NOT NULL,
	profErr real NOT NULL,
	objID bigint NOT NULL,
 CONSTRAINT pk_PhotoProfile_objID_bin_band PRIMARY KEY  
(
	objID,
	bin,
	band
)
);
CREATE TABLE USNOB(
	raErr float NOT NULL,
	decErr float NOT NULL,
	raDecCorr float NOT NULL,
	delta real NOT NULL,
	"match" int NOT NULL,
	pmL real NOT NULL,
	pmB real NOT NULL,
	pmRa real NOT NULL,
	pmDec real NOT NULL,
	pmRaErr real NOT NULL,
	pmDecErr real NOT NULL,
	sigRa real NOT NULL,
	sigDec real NOT NULL,
	nFit int NOT NULL,
	O real NOT NULL,
	E real NOT NULL,
	J real NOT NULL,
	F real NOT NULL,
	N real NOT NULL,
	dist20 real NOT NULL,
	dist22 real NOT NULL,
	objid bigint NOT NULL,
 CONSTRAINT pk_USNOB_objID PRIMARY KEY  
(
	objid
)
);
CREATE TABLE USNO(
	delta real NOT NULL,
	"match" int NOT NULL,
	propermotion real NOT NULL,
	angle real NOT NULL,
	blue real NOT NULL,
	red real NOT NULL,
	objID bigint NOT NULL,
 CONSTRAINT pk_USNO_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE PhotoAuxAll(
	objid bigint NOT NULL,
	mode tinyint NOT NULL,
	b float NOT NULL,
	l float NOT NULL,
	raErr float NOT NULL,
	decErr float NOT NULL,
	raDecCorr float NOT NULL,
 CONSTRAINT pk_PhotoAuxAll_objID PRIMARY KEY  
(
	objid
)
);
CREATE TABLE Photoz(
	pId int NOT NULL,
	rank int NOT NULL,
	version varchar(6) NOT NULL,
	class int NOT NULL,
	objID bigint NOT NULL,
	chiSq real NOT NULL,
	z real NOT NULL,
	zErr real NOT NULL,
	t real NOT NULL,
	tErr real NOT NULL,
	c_tt real NOT NULL,
	c_tz real NOT NULL,
	c_zz real NOT NULL,
	fitRadius int NOT NULL,
	fitThreshold real NOT NULL,
	quality int NOT NULL,
	dmod real NOT NULL,
	rest_ug real NOT NULL,
	rest_gr real NOT NULL,
	rest_ri real NOT NULL,
	rest_iz real NOT NULL,
	kcorr_u real NOT NULL,
	kcorr_g real NOT NULL,
	kcorr_r real NOT NULL,
	kcorr_i real NOT NULL,
	kcorr_z real NOT NULL,
	absMag_u real NOT NULL,
	absMag_g real NOT NULL,
	absMag_r real NOT NULL,
	absMag_i real NOT NULL,
	absMag_z real NOT NULL,
 CONSTRAINT pk_Photoz_objID_rank_pId PRIMARY KEY  
(
	objID,
	rank,
	pId
)
);
CREATE TABLE First(
	delta real NOT NULL,
	"match" int NOT NULL,
	peak real NOT NULL,
	integr real NOT NULL,
	rms real NOT NULL,
	major real NOT NULL,
	minor real NOT NULL,
	pa real NOT NULL,
	id int NOT NULL,
	lambda float NOT NULL,
	eta float NOT NULL,
	objID bigint NOT NULL,
 CONSTRAINT pk_First_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE Rosat(
	delta real NOT NULL,
	"match" int NOT NULL,
	cps real NOT NULL,
	hard1 real NOT NULL,
	hard2 real NOT NULL,
	extent real NOT NULL,
	exposure real NOT NULL,
	posErr real NOT NULL,
	cpsErr real NOT NULL,
	hard1Err real NOT NULL,
	hard2Err real NOT NULL,
	extentLike real NOT NULL,
	detectLike real NOT NULL,
	objID bigint NOT NULL,
 CONSTRAINT pk_Rosat_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE ObjMask(
	objID bigint NOT NULL,
	size int NOT NULL,
	nspan int NOT NULL,
	row0 int NOT NULL,
	col0 int NOT NULL,
	rmin int NOT NULL,
	rmax int NOT NULL,
	cmin int NOT NULL,
	cmax int NOT NULL,
	npix int NOT NULL,
	span text NOT NULL,
 CONSTRAINT pk_ObjMask_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE MatchHead(
	objID bigint NOT NULL,
	averageRa float NOT NULL DEFAULT (0),
	averageDec float NOT NULL DEFAULT (0),
	varRa float NOT NULL DEFAULT (0),
	varDec float NOT NULL DEFAULT (0),
	matchCount tinyint NOT NULL DEFAULT (0),
	missCount tinyint NOT NULL DEFAULT (0),
 CONSTRAINT pk_MatchHead_objID PRIMARY KEY  
(
	objID
)
);
CREATE TABLE MaskedObject(
	objid bigint NOT NULL,
	maskID bigint NOT NULL,
	maskType int NOT NULL,
 CONSTRAINT pk_MaskedObject_objid_maskid PRIMARY KEY  
(
	objid,
	maskID
)
);
CREATE TABLE Neighbors(
	objID bigint NOT NULL,
	neighborObjID bigint NOT NULL,
	distance float NOT NULL,
	type tinyint NOT NULL,
	neighborType tinyint NOT NULL,
	mode tinyint NOT NULL,
	neighborMode tinyint NOT NULL,
 CONSTRAINT pk_Neighbors_objID_NeighborObjID PRIMARY KEY  
(
	objID,
	neighborObjID
)
);
CREATE TABLE Zone(
	zoneID int NOT NULL,
	ra float NOT NULL,
	"dec" float NOT NULL,
	objID bigint NOT NULL,
	type tinyint NOT NULL,
	mode tinyint NOT NULL,
	cx float NOT NULL,
	cy float NOT NULL,
	cz float NOT NULL,
	native tinyint NOT NULL,
 CONSTRAINT pk_Zone_zoneID_ra_objID PRIMARY KEY  
(
	zoneID,
	ra,
	objID
)
);
CREATE TABLE FileGroupMap(
	tableName varchar(128) NOT NULL,
	tableFileGroup varchar(100) NOT NULL,
	indexFileGroup varchar(100) NOT NULL,
	comment varchar(4000) NULL,
 CONSTRAINT pk_FileGroupMap_tableName PRIMARY KEY  
(
	tableName
)
);
CREATE TABLE Chunk(
	chunkID bigint NOT NULL,
	skyVersion tinyint NOT NULL,
	targetVersion varchar(32) NOT NULL,
	startMu int NOT NULL,
	endMu int NOT NULL,
	stripe int NOT NULL,
	exportVersion varchar(32) NOT NULL,
	loadVersion int NOT NULL,
 CONSTRAINT pk_Chunk_chunkID PRIMARY KEY  
(
	chunkID
)
);

COMMIT;

START TRANSACTION;

ALTER TABLE "Match"  ADD  CONSTRAINT fk_Match_matchHead_MatchHead_obj FOREIGN KEY(matchHead)
REFERENCES MatchHead (objID)
;
ALTER TABLE "Match"  ADD  CONSTRAINT fk_Match_objID1_PhotoObjAll_objI FOREIGN KEY(objID1)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE SpecObjAll  ADD  CONSTRAINT fk_SpecObjAll_plateID_PlateX_pla FOREIGN KEY(plateID)
REFERENCES PlateX (plateID)
;
ALTER TABLE HoleObj  ADD  CONSTRAINT fk_HoleObj_plateID_PlateX_plateI FOREIGN KEY(plateID)
REFERENCES PlateX (plateID)
;
ALTER TABLE SpecPhotoAll  ADD  CONSTRAINT fk_SpecPhotoAll_specObjID_SpecOb FOREIGN KEY(specObjID)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE QuasarCatalog  ADD  CONSTRAINT fk_QuasarCatalog_specObjID_SpecO FOREIGN KEY(specObjId)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE SpecLineIndex  ADD  CONSTRAINT fk_SpecLineIndex_specobjID_SpecO FOREIGN KEY(specobjID)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE SpecLineAll  ADD  CONSTRAINT fk_SpecLineAll_specObjID_SpecObj FOREIGN KEY(specobjID)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE XCRedshift  ADD  CONSTRAINT fk_XCRedshift_specObjID_SpecObjA FOREIGN KEY(specObjID)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE ELRedShift  ADD  CONSTRAINT fk_ELRedshift_specObjID_SpecObjA FOREIGN KEY(specObjID)
REFERENCES SpecObjAll (specObjID)
;
ALTER TABLE TargetInfo  ADD  CONSTRAINT fk_TargetInfo_targetID_Target_ta FOREIGN KEY(targetID)
REFERENCES Target (targetID)
;
ALTER TABLE TiledTargetAll  ADD  CONSTRAINT fk_TiledTargetAll_tile_TileAll_t FOREIGN KEY(tile)
REFERENCES TileAll (tile)
;
ALTER TABLE PlateX  ADD  CONSTRAINT fk_PlateX_tile_TileAll_tile FOREIGN KEY(tile)
REFERENCES TileAll (tile)
;
ALTER TABLE Sector2Tile  ADD  CONSTRAINT fk_Sector2Tile_regionID_Region_r FOREIGN KEY(regionID)
REFERENCES Region (regionid)
;
ALTER TABLE Sector2Tile  ADD  CONSTRAINT fk_Sector2Tile_tile_TileAll_tile FOREIGN KEY(tile)
REFERENCES TileAll (tile)
;
ALTER TABLE TilingInfo  ADD  CONSTRAINT fk_TilingInfo_tileRun_TilingRun_ FOREIGN KEY(tileRun)
REFERENCES TilingRun (tileRun)
;
ALTER TABLE TileAll  ADD  CONSTRAINT fk_TileAll_tileRun_TilingRun_til FOREIGN KEY(tileRun)
REFERENCES TilingRun (tileRun)
;
ALTER TABLE TilingNote  ADD  CONSTRAINT fk_TilingNote_tileRun_TilingRun_ FOREIGN KEY(tileRun)
REFERENCES TilingRun (tileRun)
;
ALTER TABLE TilingGeometry  ADD  CONSTRAINT fk_TilingGeometry_stripe_StripeD FOREIGN KEY(stripe)
REFERENCES StripeDefs (stripe)
;
ALTER TABLE TilingGeometry  ADD  CONSTRAINT fk_TilingGeometry_tileRun_Tiling FOREIGN KEY(tileRun)
REFERENCES TilingRun (tileRun)
;
ALTER TABLE RegionConvex  ADD  CONSTRAINT fk_RegionConvex_regionID_Region_ FOREIGN KEY(regionID)
REFERENCES Region (regionid)
;
ALTER TABLE Region2Box  ADD  CONSTRAINT fk_Region2Box_boxID_Region_regio FOREIGN KEY(boxid)
REFERENCES Region (regionid)
;
ALTER TABLE Sector  ADD  CONSTRAINT fk_Sector_regionID_Region_region FOREIGN KEY(regionID)
REFERENCES Region (regionid)
;
ALTER TABLE RegionArcs  ADD  CONSTRAINT fk_RegionArcs_regionID_Region_re FOREIGN KEY(regionid)
REFERENCES Region (regionid)
;
ALTER TABLE HalfSpace  ADD  CONSTRAINT fk_HalfSpace_regionID_Region_reg FOREIGN KEY(regionid)
REFERENCES Region (regionid)
;
ALTER TABLE Inventory  ADD  CONSTRAINT fk_Inventory_name_DBObjects_name FOREIGN KEY(name)
REFERENCES DBObjects (name)
;
ALTER TABLE DBColumns  ADD  CONSTRAINT fk_DBColumns_tablename_DBObjects FOREIGN KEY(tablename)
REFERENCES DBObjects (name)
;
ALTER TABLE DBViewCols  ADD  CONSTRAINT fk_DBViewCols_viewname_DBObjects FOREIGN KEY(viewname)
REFERENCES DBObjects (name)
;
ALTER TABLE IndexMap  ADD  CONSTRAINT fk_IndexMap_tableName_DBObjects_ FOREIGN KEY(tableName)
REFERENCES DBObjects (name)
;
ALTER TABLE BestTarget2Sector  ADD  CONSTRAINT fk_BestTarget2Sector_objID_Photo FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE BestTarget2Sector  ADD  CONSTRAINT fk_BestTarget2Sector_regionID_Se FOREIGN KEY(regionID)
REFERENCES Sector (regionID)
;
ALTER TABLE Segment  ADD  CONSTRAINT fk_Segment_chunkId_Chunk_chunkId FOREIGN KEY(chunkID)
REFERENCES Chunk (chunkID)
;
ALTER TABLE Segment  ADD  CONSTRAINT fk_Segment_stripe_StripeDefs_str FOREIGN KEY(stripe)
REFERENCES StripeDefs (stripe)
;
ALTER TABLE Field  ADD  CONSTRAINT fk_Field_segmentID_Segment_segme FOREIGN KEY(segmentID)
REFERENCES Segment (segmentID)
;
ALTER TABLE PhotoObjAll  ADD  CONSTRAINT fk_PhotoObjAll_fieldID_Field_fie FOREIGN KEY(fieldID)
REFERENCES Field (fieldID)
;
ALTER TABLE RunQA  ADD  CONSTRAINT fk_RunQA_fieldID_Field_fieldID FOREIGN KEY(fieldID)
REFERENCES Field (fieldID)
;
ALTER TABLE FieldProfile  ADD  CONSTRAINT fk_FieldProfile_fieldID_Field_fi FOREIGN KEY(fieldID)
REFERENCES Field (fieldID)
;
ALTER TABLE PhotoTag  ADD  CONSTRAINT fk_PhotoTag_fieldID_Field_fieldI FOREIGN KEY(fieldID)
REFERENCES Field (fieldID)
;
ALTER TABLE PhotoTag  ADD  CONSTRAINT fk_PhotoTag_objID_PhotoObjAll_ob FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE Frame  ADD  CONSTRAINT fk_Frame_fieldID_Field_fieldID FOREIGN KEY(fieldID)
REFERENCES Field (fieldID)
;
ALTER TABLE ObjMask  ADD  CONSTRAINT fk_ObjMask_objID_PhotoObjAll_obj FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE MatchHead  ADD  CONSTRAINT fk_MatchHead_objID_PhotoObjAll_o FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE MaskedObject  ADD  CONSTRAINT fk_MaskedObject_maskID_Mask_mask FOREIGN KEY(maskID)
REFERENCES Mask (maskID)
;
ALTER TABLE MaskedObject  ADD  CONSTRAINT fk_MaskedObject_objID_PhotoObjAl FOREIGN KEY(objid)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE Neighbors  ADD  CONSTRAINT fk_Neighbors_objID_PhotoObjAll_o FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE Zone  ADD  CONSTRAINT fk_Zone_objID_PhotoObjAll_objID FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE USNOB  ADD  CONSTRAINT fk_USNOB_objID_PhotoObjAll_objID FOREIGN KEY(objid)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE USNO  ADD  CONSTRAINT fk_USNO_objID_PhotoObjAll_objID FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE PhotoAuxAll  ADD  CONSTRAINT fk_PhotoAuxAll_objID_PhotoObjAll FOREIGN KEY(objid)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE Photoz  ADD  CONSTRAINT fk_Photoz_objID_PhotoObjAll_objI FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE First  ADD  CONSTRAINT fk_First_objID_PhotoObjAll_objID FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE Rosat  ADD  CONSTRAINT fk_Rosat_objID_PhotoObjAll_objID FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE PhotoProfile  ADD  CONSTRAINT fk_PhotoProfile_objID_PhotoObjAl FOREIGN KEY(objID)
REFERENCES PhotoObjAll (objID)
;
ALTER TABLE FileGroupMap  ADD  CONSTRAINT fk_FileGroupMap_tableFileGroup_P FOREIGN KEY(tableFileGroup)
REFERENCES PartitionMap (fileGroupName)
;
ALTER TABLE Chunk  ADD  CONSTRAINT fk_Chunk_stripe_StripeDefs_strip FOREIGN KEY(stripe)
REFERENCES StripeDefs (stripe)
;

COMMIT;
