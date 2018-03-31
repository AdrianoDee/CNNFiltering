import argparse
import dataset
from keras.layers import Input, Conv2D, MaxPooling2D, Flatten, Dense
from keras.layers import concatenate, Dropout, BatchNormalization, AveragePooling2D
from keras.models import Model
from keras import optimizers
from keras.constraints import max_norm
from keras.utils import plot_model


IMAGE_SIZE = dataset.padshape


def adam_small_doublet_model(args, n_channels,n_labels=2):
    hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')

    #drop = Dropout(args.dropout)(hit_shapes)
    conv = Conv2D(32, (4, 4), activation='relu', padding='same', data_format="channels_last", name='conv1')(hit_shapes)
    conv = Conv2D(32, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv2')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool1')(conv)

    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv3')(pool)
    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv4')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool2')(conv)

    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv5')(pool)
    pool = AveragePooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='avgpool')(conv)

    flat = Flatten()(pool)
    concat = concatenate([flat, infos])

    b_norm = BatchNormalization()(concat)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(b_norm)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(32, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(n_labels, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[hit_shapes, infos], outputs=pred)
    model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])
    return model


def big_filters_model(args, n_channels):
    hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')

    conv = Conv2D(128, (5, 5), activation='relu', padding='valid', data_format="channels_last", name='conv1')(hit_shapes)

    flat = Flatten()(conv)
    concat = concatenate([flat, infos])

    drop = Dropout(args.dropout)(concat)
    dense = Dense(256, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(drop)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(2, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[hit_shapes, infos], outputs=pred)
    my_sgd = optimizers.SGD(lr=args.lr, decay=1e-5, momentum=args.momentum, nesterov=True)
    model.compile(optimizer=my_sgd, loss='categorical_crossentropy', metrics=['accuracy'])
    return model


def dense_model(args, n_channels):
    hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')
    flat = Flatten()(hit_shapes)
    concat = concatenate([flat, infos])

    b_norm = BatchNormalization()(concat)
    dense = Dense(256, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(b_norm)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(128, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense3')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(2, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[hit_shapes, infos], outputs=pred)
    my_sgd = optimizers.SGD(lr=args.lr, decay=1e-4, momentum=args.momentum, nesterov=True)
    model.compile(optimizer=my_sgd, loss='categorical_crossentropy', metrics=['accuracy'])
    return model


def small_doublet_model(args, n_channels,n_labels=2):
    hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')

#    drop = Dropout(args.dropout)(hit_shapes)
    conv = Conv2D(32, (5, 5), activation='relu', padding='same', data_format="channels_last", name='conv1')(hit_shapes)
    conv = Conv2D(32, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv2')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool1')(conv)

    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv3')(pool)
    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv4')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool2')(conv)

    flat = Flatten()(pool)
    concat = concatenate([flat, infos])

    b_norm = BatchNormalization()(concat)
    dense = Dense(128, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(b_norm)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(n_labels, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[hit_shapes, infos], outputs=pred)
    my_sgd = optimizers.SGD(lr=args.lr, decay=1e-4, momentum=args.momentum, nesterov=True)
    model.compile(optimizer=my_sgd, loss='categorical_crossentropy', metrics=['accuracy'])
    return model


def big_doublet_model(args, n_channels):
    hit_shapes = Input(shape=(8, 8, n_channels), name='hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')

    drop = Dropout(args.dropout)(hit_shapes)
    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv1')(drop)
    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv2')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool1')(conv)

    conv = Conv2D(256, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv3')(pool)
    conv = Conv2D(256, (3, 3), activation='relu', padding='same', data_format="channels_last", name='conv4')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='pool2')(conv)

    flat = Flatten()(pool)
    concat = concatenate([flat, infos])

    drop = Dropout(args.dropout)(concat)
    dense = Dense(256, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(drop)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(2, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[hit_shapes, infos], outputs=pred)
    my_sgd = optimizers.SGD(lr=args.lr, decay=1e-4, momentum=args.momentum, nesterov=True)
    model.compile(optimizer=my_sgd, loss='categorical_crossentropy', metrics=['accuracy'])
    return model


def separate_conv_doublet_model(args, n_channels):
    in_hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='in_hit_shape_input')
    out_hit_shapes = Input(shape=(IMAGE_SIZE, IMAGE_SIZE, n_channels), name='out_hit_shape_input')
    infos = Input(shape=(len(dataset.featureLabs),), name='info_input')

    # input shape convolution
    drop = Dropout(args.dropout)(in_hit_shapes)
    conv = Conv2D(64, (5, 5), activation='relu', padding='same', data_format="channels_last", name='in_conv1')(drop)
    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='in_conv2')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='in_pool1')(conv)

    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='in_conv3')(pool)
    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='in_conv4')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='in_pool2')(conv)
    in_flat = Flatten()(pool)

    # output shape convolution
    drop = Dropout(args.dropout)(out_hit_shapes)
    conv = Conv2D(64, (5, 5), activation='relu', padding='same', data_format="channels_last", name='out_conv1')(drop)
    conv = Conv2D(64, (3, 3), activation='relu', padding='same', data_format="channels_last", name='out_conv2')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='out_pool1')(conv)

    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='out_conv3')(pool)
    conv = Conv2D(128, (3, 3), activation='relu', padding='same', data_format="channels_last", name='out_conv4')(conv)
    pool = MaxPooling2D(pool_size=(2, 2), padding='same', data_format="channels_last", name='out_pool2')(conv)
    out_flat = Flatten()(pool)

    concat = concatenate([in_flat, out_flat, infos])
    info_drop = Dropout(args.dropout)(concat)

    dense = Dense(256, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense1')(info_drop)
    drop = Dropout(args.dropout)(dense)
    dense = Dense(64, activation='relu', kernel_constraint=max_norm(args.maxnorm), name='dense2')(drop)
    drop = Dropout(args.dropout)(dense)
    pred = Dense(2, activation='softmax', kernel_constraint=max_norm(args.maxnorm), name='output')(drop)

    model = Model(inputs=[in_hit_shapes, out_hit_shapes, infos], outputs=pred)
    my_sgd = optimizers.SGD(lr=args.lr, decay=1e-4, momentum=args.momentum, nesterov=True)
    model.compile(optimizer=my_sgd, loss='categorical_crossentropy', metrics=['accuracy'])
    return model

class DoubletsDataGenerator:

    def __init__(self, list_IDs, labels, batch_size=32, dim=(32,32,32), n_channels=20,
             n_classes=2, shuffle=True):
             """Generator Definition"""
             self.dim = dim
             self.batch_size = batch_size
             self.labels = labels
             self.list_IDs = list_IDs
             self.n_channels = n_channels
             self.n_classes = n_classes
             self.shuffle = shuffle
             self.on_epoch_end()

    def on_epoch_end(self):
        '''Updates indexes after each epoch'''
        self.indexes = np.arange(len(self.list_IDs))
        if self.shuffle == True:
            np.random.shuffle(self.indexes)

    def __data_generation(self, list_IDs_temp):
  '''Generates data containing batch_size samples''' # X : (n_samples, *dim, n_channels)
        # Initialization
        X = np.empty((self.batch_size, *self.dim, self.n_channels))
        y = np.empty((self.batch_size), dtype=int)

        # Generate data
        for i, ID in enumerate(list_IDs_temp):
            # Store sample
            X[i,] = np.load('data/' + ID + '.npy')

            # Store class
            y[i] = self.labels[ID]

        return X, keras.utils.to_categorical(y, num_classes=self.n_classes)

    def __len__(self):
  '''Denotes the number of batches per epoch'''
  return int(np.floor(len(self.list_IDs) / self.batch_size))
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--n_epochs', type=int, default=100)
    parser.add_argument('--batch_size', type=int, default=128)
    parser.add_argument('--dropout', type=float, default=0.5)
    parser.add_argument('--lr', type=float, default=0.001, help='learning rate')
    parser.add_argument('--momentum', type=float, default=0.5)
    parser.add_argument('--patience', type=int, default=15)
    parser.add_argument('--log_dir', type=str, default="models/cnn_doublet")
    parser.add_argument('--name', type=str, default='model_')
    parser.add_argument('--maxnorm', type=float, default=10.)
    parser.add_argument('--verbose', type=int, default=1)
    main_args = parser.parse_args()

    plot_model(big_doublet_model(main_args, 8), to_file='big_model.png', show_shapes=True, show_layer_names=True)
    plot_model(separate_conv_doublet_model(main_args, 4), to_file='separate_conv_model.png', show_shapes=True, show_layer_names=True)
