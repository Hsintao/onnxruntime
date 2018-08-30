"""
Train, convert and predict with Lotus
=====================================

This example demonstrates an end to end scenario
starting with the training of a scikit-learn pipeline
which takes as inputs not a regular vector but a
dictionary ``{ int: float }`` as its first step is a
`DictVectorizer <http://scikit-learn.org/stable/modules/generated/sklearn.feature_extraction.DictVectorizer.html>`_.

.. contents::
    :local:

Train a pipeline
++++++++++++++++

The first step consists in retrieving the boston datset.
"""
import pandas
from sklearn.datasets import load_boston
boston = load_boston()
X, y = boston.data, boston.target

from sklearn.model_selection import train_test_split
X_train, X_test, y_train, y_test = train_test_split(X, y)
X_train_dict = pandas.DataFrame(X_train[:,1:]).T.to_dict().values()
X_test_dict = pandas.DataFrame(X_test[:,1:]).T.to_dict().values()

####################################
# We create a pipeline.

from sklearn.pipeline import make_pipeline
from sklearn.ensemble import GradientBoostingRegressor
from sklearn.feature_extraction import DictVectorizer
pipe = make_pipeline(
            DictVectorizer(sparse=False),
            GradientBoostingRegressor())
            
pipe.fit(X_train_dict, y_train)

####################################
# We compute the prediction on the test set
# and we show the confusion matrix.
from sklearn.metrics import r2_score

pred = pipe.predict(X_test_dict)
print(r2_score(y_test, pred))

####################################
# Conversion to ONNX format
# +++++++++++++++++++++++++
#
# We use module 
# `onnxmltools <https://github.com/onnx/onnxmltools>`_
# to convert the model into ONNX format.

from onnxmltools import convert_sklearn
from onnxmltools.utils import save_model
from onnxmltools.convert.common.data_types import FloatTensorType, Int64TensorType, DictionaryType, SequenceType

# initial_type = [('float_input', DictionaryType(Int64TensorType([1]), FloatTensorType([])))]
initial_type = [('float_input', DictionaryType(Int64TensorType([1]), FloatTensorType([])))]
onx = convert_sklearn(pipe, initial_types=initial_type)
save_model(onx, "pipeline_vectorize.onnx")

##################################
# We load the model with Lotus and look at
# its input and output.
import lotus
sess = lotus.InferenceSession("pipeline_vectorize.onnx")

import numpy
inp, out = sess.get_inputs()[0], sess.get_outputs()[0]
print("input name='{}' and shape={} and type={}".format(inp.name, inp.shape, inp.type))
print("output name='{}' and shape={} and type={}".format(out.name, out.shape, out.type))

##################################
# We compute the predictions one by one.

input_name = sess.get_inputs()[0].name
label_name = sess.get_outputs()[0].name

##############################
# We could do that...

try:
    pred_onx = sess.run([label_name], {input_name: X_test_dict})[0]
except RuntimeError as e:
    print(e)

#############################
#  But it fails because, in case of a DictVectorizer,
# Lotus expects one observation at a time.
pred_onx = [sess.run([label_name], {input_name: row})[0][0, 0] for row in X_test_dict]

###############################
# We compare them to the model's ones.
print(r2_score(pred, pred_onx))

#########################
# Very similar. Lotus uses float instead of doubles,
# that explains the small discrepencies.

