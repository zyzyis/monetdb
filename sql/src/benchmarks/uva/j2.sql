SELECT DISTINCT segment0.id
FROM VxSegment_tab segment0, 
   VxSegmentation_tab segmentation0, 
   VxSegment_tab segment1, 
   VxSegmentation_tab segmentation1, 
   VxSegmentStringFeature_tab feat1_0
WHERE (segment0.segmentationId = segmentation0.id) AND
   (segmentation0.name = 'blocks') AND
   (segmentation1.videoName = segmentation0.videoName) AND
   (segment1.segmentationId = segmentation1.id) AND
   (segmentation1.name = 'person') AND
   (segment0.startPos <= segment1.endPos) AND
   (segment1.startPos <= segment0.endPos) AND
   (feat1_0.segmentId = segment1.id) AND
   (feat1_0.fieldName = 'Person') AND
   (feat1_0.value = 'A. Huizinga');

