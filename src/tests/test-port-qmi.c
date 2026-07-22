/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <glib.h>

#include <ModemManager.h>
#include <ModemManager-tags.h>
#define _LIBMM_INSIDE_MM
#include <libmm-glib.h>

#include "mm-kernel-device-generic.h"
#include "mm-log-test.h"
#include "mm-port-net.h"
#include "mm-port-qmi.h"

typedef struct {
    const gchar *value;
    guint        expected;
} FixedMuxIdTest;

static const FixedMuxIdTest fixed_mux_id_tests[] = {
    { NULL,      QMI_DEVICE_MUX_ID_UNBOUND },
    { "0",       QMI_DEVICE_MUX_ID_UNBOUND },
    { "1",       1 },
    { "254",     254 },
    { "255",     QMI_DEVICE_MUX_ID_UNBOUND },
    { "-1",      QMI_DEVICE_MUX_ID_UNBOUND },
    { "invalid", QMI_DEVICE_MUX_ID_UNBOUND },
};

typedef struct {
    const gchar *value;
    gboolean     expected;
} DefaultMultiplexTest;

static const DefaultMultiplexTest default_multiplex_tests[] = {
    { NULL,      FALSE },
    { "0",       FALSE },
    { "false",   FALSE },
    { "1",       TRUE },
    { "true",    TRUE },
};

static void
test_fixed_mux_id (void)
{
    guint i;

    for (i = 0; i < G_N_ELEMENTS (fixed_mux_id_tests); i++) {
        g_autoptr(MMKernelDevice)          kernel_device = NULL;
        g_autoptr(MMKernelEventProperties) properties = NULL;
        g_autoptr(MMPortNet)               net = NULL;
        g_autoptr(MMPortQmi)               qmi = NULL;
        g_autoptr(GError)                  error = NULL;

        properties = mm_kernel_event_properties_new ();
        mm_kernel_event_properties_set_action (properties, "add");
        mm_kernel_event_properties_set_subsystem (properties, "virtual");
        mm_kernel_event_properties_set_name (properties, "test-net");

        kernel_device = mm_kernel_device_generic_new_with_rules (properties, NULL, &error);
        g_assert_no_error (error);
        g_assert (kernel_device);

        if (fixed_mux_id_tests[i].value)
            g_object_set_data_full (G_OBJECT (kernel_device),
                                    ID_MM_QMI_FIXED_MUX_ID,
                                    g_strdup (fixed_mux_id_tests[i].value),
                                    g_free);

        net = mm_port_net_new ("test-net");
        g_object_set (net,
                      MM_PORT_KERNEL_DEVICE, kernel_device,
                      NULL);

        qmi = mm_port_qmi_new ("test-qmi", MM_PORT_SUBSYS_USBMISC);
        mm_port_qmi_set_net_details (qmi, MM_PORT (net));

        g_assert_cmpuint (mm_port_qmi_get_fixed_mux_id (qmi), ==, fixed_mux_id_tests[i].expected);
    }
}

static void
test_default_multiplex (void)
{
    guint i;

    for (i = 0; i < G_N_ELEMENTS (default_multiplex_tests); i++) {
        g_autoptr(MMKernelDevice)          kernel_device = NULL;
        g_autoptr(MMKernelEventProperties) properties = NULL;
        g_autoptr(MMPortNet)               net = NULL;
        g_autoptr(MMPortQmi)               qmi = NULL;
        g_autoptr(GError)                  error = NULL;

        properties = mm_kernel_event_properties_new ();
        mm_kernel_event_properties_set_action (properties, "add");
        mm_kernel_event_properties_set_subsystem (properties, "virtual");
        mm_kernel_event_properties_set_name (properties, "test-net");

        kernel_device = mm_kernel_device_generic_new_with_rules (properties, NULL, &error);
        g_assert_no_error (error);
        g_assert (kernel_device);

        if (default_multiplex_tests[i].value)
            g_object_set_data_full (G_OBJECT (kernel_device),
                                    ID_MM_QMI_DEFAULT_MULTIPLEX,
                                    g_strdup (default_multiplex_tests[i].value),
                                    g_free);

        net = mm_port_net_new ("test-net");
        g_object_set (net,
                      MM_PORT_KERNEL_DEVICE, kernel_device,
                      NULL);

        qmi = mm_port_qmi_new ("test-qmi", MM_PORT_SUBSYS_USBMISC);
        mm_port_qmi_set_net_details (qmi, MM_PORT (net));

        g_assert_cmpint (mm_port_qmi_get_default_multiplex (qmi), ==, default_multiplex_tests[i].expected);
    }
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/MM/port-qmi/fixed-mux-id", test_fixed_mux_id);
    g_test_add_func ("/MM/port-qmi/default-multiplex", test_default_multiplex);

    return g_test_run ();
}
